#include "PrtyMPsiReceiver.h"

#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Crypto/Commit.h>
#include <cryptoTools/Network/Channel.h>
#include "libOTe/TwoChooseOne/IknpOtExtSender.h"
#include "Poly/polyFFT.h"
#include "Tools/GaussianElimination.h"



using namespace std;
using namespace NTL;

namespace osuCrypto
{
	void PrtyMPsiReceiver::init(u64 myInputSize, u64 theirInputSize, u64 psiSecParam, PRNG& prng, span<Channel> chls, bool isMalicious)
	{
		mPsiSecParam = psiSecParam;
		mMyInputSize = myInputSize;
		mTheirInputSize = theirInputSize;
		mMaskLength = 64 / 8;// (psiSecParam + log2(mTheirInputSize * mMyInputSize) + 7) / 8;

		mPrng.SetSeed(prng.get<block>());

		if (isMalicious)
			mFieldSize = 132;// getMalCodewordSize(myInputSize);
		else
			mFieldSize = 132;//getShCodewordSize(myInputSize);

		mNumBin = getBinSize(mTheirInputSize); //TODO: remove
		mSigma = getSigma(mTheirInputSize);

		mNumOTs = mNumBin + mSigma;
		mPrytOtRecv.configure(isMalicious, psiSecParam, mFieldSize);


		std::vector<block> baseOtRecv(128);
		BitVector baseOtChoices(128);
		baseOtChoices.randomize(mPrng);
		NaorPinkas baseOTs;
		baseOTs.receive(baseOtChoices, baseOtRecv, mPrng, chls[0], chls.size());


		auto matrixWidth = mPrytOtRecv.mGens.size();
		IknpOtExtSender sendIKNP;
		sendIKNP.setBaseOts(baseOtRecv, baseOtChoices);

		std::vector<std::array<block, 2>>  baseSend(matrixWidth);
		sendIKNP.send(baseSend, mPrng, chls[0]);


		mPrytOtRecv.setBaseOts(baseSend);
		mPrytOtRecv.init(mNumOTs, mPrng, chls[0]);
	}

	void PrtyMPsiReceiver::output(span<block> inputs, span<Channel> chls)
	{
		std::vector<std::thread> thrds(chls.size());
		const bool isMultiThreaded = chls.size() > 1;
		std::mutex mtx;

		std::vector<std::array<block, prty2SuperBlkSize>> yInputs(inputs.size()), cuckooTables;
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			for (int k = 0; k < prty2SuperBlkSize; k++)
				yInputs[idxItem][k] = inputs[idxItem];  // fake H1(x)

		std::unordered_map<u64, std::pair<block, u64>> localMasks;
		localMasks.reserve(mMyInputSize);

		// Generate a cuckoo table
		Cuckoo_encode(inputs, yInputs, cuckooTables, mNumBin, mSigma); //single thread


		//==================OOS
		auto oos_routine = [&](u64 t)
		{
			auto& chl = chls[t];
			u64 startOtIdx = mNumOTs * t / thrds.size();
			u64 tempEndOtIdx = (mNumOTs * (t + 1) / thrds.size());
			u64 endOtIdx = std::min(tempEndOtIdx, mNumOTs);

			for (u64 i = startOtIdx; i < endOtIdx; i += stepSize)
			{
				auto curStepSize = std::min<u64>(stepSize, mNumOTs - i);
				for (u64 k = 0; k < curStepSize; ++k)
					mPrytOtRecv.otCorrection(i + k, &cuckooTables[k + i]);

				mPrytOtRecv.sendCorrection(chl, curStepSize);
			}
		};

		for (u64 i = 0; i < thrds.size(); ++i)
		{
			thrds[i] = std::thread([=] {
				oos_routine(i);
				});
		}

		for (auto& thrd : thrds)
			thrd.join();

		//=========compute PSI last message
		Cuckoo_decode(inputs, mPrytOtRecv.mRy, mPrytOtRecv.mT0, mNumBin, mSigma); //Decode(R,y) 

		auto psi_routine = [&](u64 t)
		{
			auto& chl = chls[t];
			u64 startIdx = mMyInputSize * t / thrds.size();
			u64 tempEndIdx = (mMyInputSize * (t + 1) / thrds.size());
			u64 endIdx = std::min(tempEndIdx, mMyInputSize);
			block prtyEncoding2;

			for (u64 i = startIdx; i < endIdx; i += stepSize)
			{
				auto curStepSize = std::min<u64>(stepSize, inputs.size() - i);
				for (u64 k = 0; k < curStepSize; ++k)
				{
					// compute prtyEncoding1=H2(y, Decode(R,y))
					mPrytOtRecv.encode_prty(i + k, &inputs[k + i], (u8*)& prtyEncoding2, sizeof(block));

					//std::cout << IoStream::lock << "prtyEncoding2: " << prtyEncoding2 << std::endl << IoStream::unlock;


					if (isMultiThreaded)
					{
						std::lock_guard<std::mutex> lock(mtx);
						localMasks.emplace(*(u64*)& prtyEncoding2, std::pair<block, u64>(prtyEncoding2, i+k));
					}
					else
						localMasks.emplace(*(u64*)& prtyEncoding2, std::pair<block, u64>(prtyEncoding2, i + k));
				}
			}

		};



		for (u64 i = 0; i < thrds.size(); ++i)
		{
			thrds[i] = std::thread([=] {
				psi_routine(i);
				});
		}

		for (auto& thrd : thrds)
			thrd.join();


		/*for (auto match = localMasks.begin(); match != localMasks.end(); ++match)
		{
			std::cout << IoStream::lock << "r match->first: " << match->first<< std::endl << IoStream::unlock;
		}*/

		//======compute PSI
		auto output_psi = [&](u64 t)
		{
			auto& chl = chls[t];
			u64 startTheirIdx = mTheirInputSize * t / thrds.size();
			u64 tempEndTheirIdx = (mTheirInputSize * (t + 1) / thrds.size());
			u64 endTheirIdx = std::min(tempEndTheirIdx, mTheirInputSize);

			std::vector<u8> recvBuffs;

			for (u64 i = startTheirIdx; i < endTheirIdx; i += stepSize)
			{
				auto curStepSize = std::min<u64>(stepSize, mTheirInputSize - i);

				chl.recv(recvBuffs); //receive Hash

				//std::cout << IoStream::lock << "r recvBuffs: " << recvBuffs.size() << std::endl << IoStream::unlock;

				auto theirMasks = recvBuffs.data();

			/*	block aa;
				memcpy((u8*)& aa, recvBuffs.data(), sizeof(block));

				std::cout << IoStream::lock << "r theirMasks: " << aa << std::endl << IoStream::unlock;*/

				if (mMaskLength >= sizeof(u64))
				{
					for (u64 k = 0; k < curStepSize; ++k)
					{
						//auto match = localMasks.find(*(u64*)& theirMasks);

						auto& msk = *(u64*)(theirMasks);
						// check 64 first bits
						auto match = localMasks.find(msk);

					//	std::cout << IoStream::lock << "r msk: " << msk << std::endl << IoStream::unlock;

						if (match != localMasks.end())//if match, check for whole bits
						{
						//	std::cout << IoStream::lock << " match != localMasks.end()" << std::endl << IoStream::unlock;

							if (memcmp((u8*)& theirMasks, &match->second.first, mMaskLength)) // check full mask
							{
								if (isMultiThreaded)
								{
									std::lock_guard<std::mutex> lock(mtx);
									mIntersection.push_back(match->second.second);
								}
								else
									mIntersection.push_back(match->second.second);

							//	std::cout << "r mask: " << match->second.first << "\n";
							}
						}
						theirMasks += mMaskLength;
					}
				}
				else //for small set, do O(n^2) check
				{
					for (u64 k = 0; k < curStepSize; ++k)
					{
						for (auto match = localMasks.begin(); match != localMasks.end(); ++match)
						{
							if (memcmp(theirMasks, &match->second.first, mMaskLength)) // check full mask
							{
								if (isMultiThreaded)
								{
									std::lock_guard<std::mutex> lock(mtx);
									mIntersection.push_back(match->second.second);
								}
								else
									mIntersection.push_back(match->second.second);
							}

						}
						theirMasks += mMaskLength;
					}
				}



			}

		};

		for (u64 i = 0; i < thrds.size(); ++i)
		{
			thrds[i] = std::thread([=] {
				output_psi(i);
				});
		}

		for (auto& thrd : thrds)
			thrd.join();

	}

}
