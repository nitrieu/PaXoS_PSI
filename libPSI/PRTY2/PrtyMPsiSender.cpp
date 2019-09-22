#include "PrtyMPsiSender.h"
#include <cryptoTools/Crypto/Commit.h>
#include <cryptoTools/Common/Timer.h>
#include "libOTe/Base/naor-pinkas.h"
#include <unordered_map>
#include "PrtyMOtSender.h"
#include "Tools/GaussianElimination.h"

namespace osuCrypto
{
    using namespace std;
	using namespace NTL;


	void PrtyMPsiSender::init(u64 myInputSize, u64 theirInputSize, u64 psiSecParam, PRNG & prng, span<Channel> chls, bool isMalicious)
	{
		mPsiSecParam = psiSecParam;
		mMyInputSize = myInputSize;
		mTheirInputSize = theirInputSize;
		mMaskLength = (psiSecParam + log2(mTheirInputSize * mMyInputSize) + 7) / 8;
		mIsMalicious = isMalicious;


		mPrng.SetSeed(prng.get<block>());
		if (isMalicious)
			mFieldSize = 132;// getMalCodewordSize(mTheirInputSize);
		else
			mFieldSize = 132;//getShCodewordSize(mTheirInputSize);

		mNumBin = getBinSize(mTheirInputSize); //TODO: remove
		mSigma = getSigma(mTheirInputSize);

		mNumOTs = mNumBin + mSigma;

		mPrytOtSender.configure(isMalicious, psiSecParam, mFieldSize);

		std::vector<std::array<block, 2>> baseOtSend(128);
		NaorPinkas baseOTs;
		baseOTs.send(baseOtSend, mPrng, chls[0], chls.size());


		//IKNP for our base OT extension
		auto matrixWidth = mPrytOtSender.mGens.size();
		IknpOtExtReceiver recvIKNP;
		recvIKNP.setBaseOts(baseOtSend);
		mOtChoices.resize(matrixWidth);
		mOtChoices.randomize(mPrng);
		std::vector<block> baseRecv(matrixWidth);
		recvIKNP.receive(mOtChoices, baseRecv, mPrng, chls[0]);

		//set ~4*k base OT
		mPrytOtSender.setBaseOts(baseRecv, mOtChoices);

		//number OT we need
		mPrytOtSender.init(mNumOTs, mPrng, chls[0]);
	}

	void PrtyMPsiSender::output(span<block> inputs, span<Channel> chls)
	{
		std::vector<std::thread> thrds(chls.size());
		const bool isMultiThreaded = chls.size() > 1;
		std::mutex mtx;

		std::vector<std::array<block, prty2SuperBlkSize>> yInputs(inputs.size());
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			for (int k = 0; k < prty2SuperBlkSize; k++)
				yInputs[idxItem][k] = inputs[idxItem];  // fake H1(y)

		
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
				mPrytOtSender.recvCorrection(chl, curStepSize);  //sigle thread now
				for (u64 k = 0; k < curStepSize; ++k)
				{
					mPrytOtSender.otCorrection(i + k);
				}
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

		gTimer.setTimePoint("s_oos");

		//TODO
		/*if(mIsMalicious)
			mPrytOtSender.check(chls[0], mPrng.get<block>());*/

		//Decode(Q,inputs)
		Cuckoo_decode(inputs, mPrytOtSender.mQx, mPrytOtSender.mT, mNumBin, mSigma); //geting Decode(Q,x)

		gTimer.setTimePoint("s_Cuckoo_decode");

			//======compute PSI
		auto psi_routine = [&](u64 t)
		{
			auto& chl = chls[t];
			u64 startIdx = mMyInputSize * t / thrds.size();
			u64 tempEndIdx = (mMyInputSize * (t + 1) / thrds.size());
			u64 endIdx = std::min(tempEndIdx, mMyInputSize);
			block prtyEncoding1;

			for (u64 i = startIdx; i < endIdx; i += stepSize)
			{
				auto curStepSize = std::min<u64>(stepSize, inputs.size() - i);
				std::vector<u8> sendBuff(curStepSize * mMaskLength);

				for (u64 k = 0; k < curStepSize; ++k)
				{
					//compute prtyEncoding1= H2(x, Decode(Q,x) +C(H1(x))*s)
					mPrytOtSender.encode_prty(i + k, &yInputs[k + i], (u8*)& prtyEncoding1, sizeof(block));
					
					//std::cout << IoStream::lock << "prtyEncoding1: " << prtyEncoding1 << std::endl << IoStream::unlock;

					memcpy(sendBuff.data()+ k*mMaskLength, (u8*)& prtyEncoding1, mMaskLength);
				}
				//block aa;
				//memcpy((u8*)& aa, sendBuff.data(), sizeof(block));
				//std::cout << IoStream::lock << "ss theirMasks: " << aa << std::endl << IoStream::unlock;
				chl.asyncSend(std::move(sendBuff));

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
	}


}

