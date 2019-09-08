#include "EcdhPsiSender.h"
#include "cryptoTools/Crypto/Curve.h"
#include "cryptoTools/Crypto/RandomOracle.h"
#include "cryptoTools/Common/Log.h"
#include "cryptoTools/Network/Channel.h"

namespace osuCrypto
{

    EcdhPsiSender::EcdhPsiSender()
    {
    }


    EcdhPsiSender::~EcdhPsiSender()
    {
    }
    void EcdhPsiSender::init(u64 n, u64 secParam, block seed)
    {
        mN = n;
        mSecParam = secParam;
        mPrng.SetSeed(seed);
    }


    void EcdhPsiSender::sendInput_k283(std::vector<block>& inputs, span<Channel> chls)
    {
		std::cout << "curveParam = k283\n";

        auto curveParam = k283;

        u64 theirInputSize = inputs.size();

		u64 maskSizeByte = (40 + 2*log2(inputs.size())+7) / 8;

        std::vector<PRNG> thrdPrng(chls.size());
        for (u64 i = 0; i < thrdPrng.size(); i++)
            thrdPrng[i].SetSeed(mPrng.get<block>());

        auto RsSeed = mPrng.get<block>();

		std::vector<std::vector<u8>> sendBuff2(chls.size());

		

	


        auto routine = [&](u64 t)
        {
            u64 inputStartIdx = inputs.size() * t / chls.size();
            u64 inputEndIdx = inputs.size() * (t + 1) / chls.size();
            u64 subsetInputSize = inputEndIdx - inputStartIdx;

			
            auto& chl = chls[t];
            auto& prng = thrdPrng[t];

			EllipticCurve curve(curveParam, thrdPrng[t].get<block>());
			RandomOracle inputHasher(sizeof(block));
			EccNumber a(curve);
			EccPoint xa(curve), point(curve), yb(curve), yba(curve);
			a.randomize(RsSeed);

			sendBuff2[t].resize(maskSizeByte * subsetInputSize);
			auto sendIter2 = sendBuff2[t].data();

			for (u64 i = inputStartIdx; i < inputEndIdx; i += stepSize)
			{
				auto curStepSize = std::min(stepSize, inputEndIdx - i);

				std::vector<u8> sendBuff(xa.sizeBytes() * curStepSize);
				auto sendIter = sendBuff.data();

				//send H(x)^a
				for (u64 k = 0; k < curStepSize; ++k)
				{
					block seed;
					inputHasher.Reset();
					inputHasher.Update(inputs[i + k]);
					inputHasher.Final(seed);

					point.randomize(seed);
					//std::cout << "sp  " << point << "  " << toBlock(hashOut) << std::endl;

					xa = (point * a);
#ifdef PRINT
					if (i == 0)
						std::cout << "xa[" << i << "] " << xa << std::endl;
#endif	
					xa.toBytes(sendIter);
					sendIter += xa.sizeBytes();
				}
				chl.asyncSend(std::move(sendBuff));	//send H(x)^a
			}


			for (u64 i = inputStartIdx; i < inputEndIdx; i += stepSize)
			{
				auto curStepSize = std::min(stepSize, inputEndIdx - i);

			

				std::vector<u8> recvBuff(yb.sizeBytes() * curStepSize);
				std::vector<u8> temp(yba.sizeBytes());

				//recv H(y)^b
				chl.recv(recvBuff);

				if (recvBuff.size() != curStepSize * yb.sizeBytes())
				{
					std::cout << "error @ " << (LOCATION) << std::endl;
					throw std::runtime_error(LOCATION);
				}
				auto recvIter = recvBuff.data();


				//send H(y)^b^a
				for (u64 k = 0; k < curStepSize; ++k)
				{
					yb.fromBytes(recvIter); recvIter += yb.sizeBytes();
					yba = yb*a;


					yba.toBytes(temp.data());
					RandomOracle ro(sizeof(block));
					ro.Update(temp.data(), temp.size());
					block blk;
					ro.Final(blk);
					memcpy(sendIter2, &blk, maskSizeByte);
#ifdef PRINT
					if (i == 0)
					{
						std::cout << "yba[" << i << "] " << yba << std::endl;
						std::cout << "temp[" << i << "] " << toBlock(temp) << std::endl;
						std::cout << "sendIter2[" << i << "] " << toBlock(sendIter2) << std::endl;
					}
#endif
					sendIter2 += maskSizeByte;
				}
				//std::cout << "dones send H(y)^b^a" << std::endl;
			}
      
			//chl.asyncSend(std::move(sendBuff2[t]));


			};


        std::vector<std::thread> thrds(chls.size());
        for (u64 i = 0; i < u64(chls.size()); ++i)
        {
            thrds[i] = std::thread([=] {
                routine(i);
            });
        }


        for (auto& thrd : thrds)
            thrd.join();

		for (u64 i = 0; i < u64(chls.size()); ++i)
		{
			thrds[i] = std::thread([=] {
				auto& chl = chls[i];
				chl.asyncSend(std::move(sendBuff2[i]));
			});
		}


		for (auto& thrd : thrds)
			thrd.join();

		//std::cout << "S done" << std::endl;

    }

	void EcdhPsiSender::sendInput_Curve25519(std::vector<block>& inputs, span<Channel> chls)
	{
		std::cout << "curveParam = Curve25519\n";

		auto curveParam = Curve25519;

		u64 theirInputSize = inputs.size();

		u64 maskSizeByte = (40 + 2 * log2(inputs.size()) + 7) / 8;

		std::vector<PRNG> thrdPrng(chls.size());
		for (u64 i = 0; i < thrdPrng.size(); i++)
			thrdPrng[i].SetSeed(mPrng.get<block>());

		auto RsSeed = mPrng.get<block>();

		std::vector<std::vector<u8>> sendBuff2(chls.size());






		auto routine = [&](u64 t)
		{
			u64 inputStartIdx = inputs.size() * t / chls.size();
			u64 inputEndIdx = inputs.size() * (t + 1) / chls.size();
			u64 subsetInputSize = inputEndIdx - inputStartIdx;


			auto& chl = chls[t];
			auto& prng = thrdPrng[t];

			EllipticCurve curve(curveParam, thrdPrng[t].get<block>());
			RandomOracle inputHasher(sizeof(block));
			EccNumber a(curve);
			EccPoint xa(curve), point(curve), yb(curve), yba(curve);
			a.randomize(RsSeed);

			sendBuff2[t].resize(maskSizeByte * subsetInputSize);
			auto sendIter2 = sendBuff2[t].data();

			for (u64 i = inputStartIdx; i < inputEndIdx; i += stepSize)
			{
				auto curStepSize = std::min(stepSize, inputEndIdx - i);

				std::vector<u8> sendBuff(xa.sizeBytes() * curStepSize);
				auto sendIter = sendBuff.data();

				//send H(x)^a
				for (u64 k = 0; k < curStepSize; ++k)
				{
					block seed;
					inputHasher.Reset();
					inputHasher.Update(inputs[i + k]);
					inputHasher.Final(seed);

					point.randomize(seed);
					//std::cout << "sp  " << point << "  " << toBlock(hashOut) << std::endl;

					xa = (point * a);
#ifdef PRINT
					if (i == 0)
						std::cout << "xa[" << i << "] " << xa << std::endl;
#endif	
					xa.toBytes(sendIter);
					sendIter += xa.sizeBytes();
				}
				chl.asyncSend(std::move(sendBuff));	//send H(x)^a
			}


			for (u64 i = inputStartIdx; i < inputEndIdx; i += stepSize)
			{
				auto curStepSize = std::min(stepSize, inputEndIdx - i);



				std::vector<u8> recvBuff(yb.sizeBytes() * curStepSize);
				std::vector<u8> temp(yba.sizeBytes());

				//recv H(y)^b
				chl.recv(recvBuff);

				if (recvBuff.size() != curStepSize * yb.sizeBytes())
				{
					std::cout << "error @ " << (LOCATION) << std::endl;
					throw std::runtime_error(LOCATION);
				}
				auto recvIter = recvBuff.data();


				//send H(y)^b^a
				for (u64 k = 0; k < curStepSize; ++k)
				{
					yb.fromBytes(recvIter); recvIter += yb.sizeBytes();
					yba = yb * a;


					yba.toBytes(temp.data());
					RandomOracle ro(sizeof(block));
					ro.Update(temp.data(), temp.size());
					block blk;
					ro.Final(blk);
					memcpy(sendIter2, &blk, maskSizeByte);
#ifdef PRINT
					if (i == 0)
					{
						std::cout << "yba[" << i << "] " << yba << std::endl;
						std::cout << "temp[" << i << "] " << toBlock(temp) << std::endl;
						std::cout << "sendIter2[" << i << "] " << toBlock(sendIter2) << std::endl;
					}
#endif
					sendIter2 += maskSizeByte;
				}
				//std::cout << "dones send H(y)^b^a" << std::endl;
			}

			//chl.asyncSend(std::move(sendBuff2[t]));


		};


		std::vector<std::thread> thrds(chls.size());
		for (u64 i = 0; i < u64(chls.size()); ++i)
		{
			thrds[i] = std::thread([=] {
				routine(i);
			});
		}


		for (auto& thrd : thrds)
			thrd.join();

		for (u64 i = 0; i < u64(chls.size()); ++i)
		{
			thrds[i] = std::thread([=] {
				auto& chl = chls[i];
				chl.asyncSend(std::move(sendBuff2[i]));
			});
		}


		for (auto& thrd : thrds)
			thrd.join();

		//std::cout << "S done" << std::endl;

	}

	void EcdhPsiSender::sendInput(std::vector<block>& inputs, span<Channel> chl, int curveType)
	{
		if (curveType == 0)
			sendInput_k283(inputs, chl);
		else
			sendInput_Curve25519(inputs, chl);
	}

	

}