#include <iostream>

//using namespace std;
#include "tests_cryptoTools/UnitTests.h"
#include "libOTe_Tests/UnitTests.h"
#include <cryptoTools/gsl/span>

#include <cryptoTools/Common/Matrix.h>

#include <cryptoTools/Common/Defines.h>
using namespace osuCrypto;

#include "libOTe/TwoChooseOne/KosOtExtReceiver.h"
#include "libOTe/TwoChooseOne/KosOtExtSender.h"
#include "libOTe/TwoChooseOne/KosDotExtReceiver.h"
#include "libOTe/TwoChooseOne/KosDotExtSender.h"

#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/IOService.h>
#include <numeric>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/Log.h>


#include "libOTe/Tools/LinearCode.h"
#include "libOTe/Tools/bch511.h"
#include "libOTe/NChooseOne/Oos/OosNcoOtReceiver.h"
#include "libOTe/NChooseOne/Oos/OosNcoOtSender.h"
#include "libOTe/NChooseOne/Kkrt/KkrtNcoOtReceiver.h"
#include "libOTe/NChooseOne/Kkrt/KkrtNcoOtSender.h"

#include "libOTe/TwoChooseOne/IknpOtExtReceiver.h"
#include "libOTe/TwoChooseOne/IknpOtExtSender.h"

#include "libOTe/NChooseK/AknOtReceiver.h"
#include "libOTe/NChooseK/AknOtSender.h"
#include "libOTe/TwoChooseOne/LzKosOtExtReceiver.h"
#include "libOTe/TwoChooseOne/LzKosOtExtSender.h"

#include "CLP.h"
#include "main.h"

#include "libOTe/TwoChooseOne/OTExtInterface.h"

#include "libOTe/Tools/Tools.h"
#include "libOTe/Tools/LinearCode.h"
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Common/Log.h>

#include "libOTe/TwoChooseOne/IknpOtExtReceiver.h"
#include "libOTe/TwoChooseOne/IknpOtExtSender.h"

#include "libOTe/TwoChooseOne/KosOtExtReceiver.h"
#include "libOTe/TwoChooseOne/KosOtExtSender.h"

#include "libOTe/TwoChooseOne/LzKosOtExtReceiver.h"
#include "libOTe/TwoChooseOne/LzKosOtExtSender.h"

#include "libOTe/TwoChooseOne/KosDotExtReceiver.h"
#include "libOTe/TwoChooseOne/KosDotExtSender.h"

#include "libOTe/NChooseOne/Kkrt/KkrtNcoOtReceiver.h"
#include "libOTe/NChooseOne/Kkrt/KkrtNcoOtSender.h"
#include "Poly/polyNTL.h"
#include "PsiDefines.h"

#include "PRTY/PrtySender.h"
#include "PRTY/PrtyReceiver.h"
#include "PRTY2/PrtyMPsiSender.h"
#include "PRTY2/PrtyMPsiReceiver.h"
#include "Tools/BalancedIndex.h"
#include "Tools/CuckooHasher.h"

#include <thread>
#include <vector>
#include <stdarg.h> 
#include "ecdhMain.h"
#include "cryptoTools/Crypto/Curve.h"


template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

static u64 expectedIntersection = 100;
u64 protocolId = 0; //bin 
//u64 protocolId = 1;  //sender.outputBigPoly(inputs, sendChls);


void usage(const char* argv0)
{
	std::cout << "Error! Please use:" << std::endl;
	std::cout << "\t 1. For unit test (balanced PSI): " << argv0 << " -t" << std::endl;
	std::cout << "\t 2. For simulation (2 terminals): " << std::endl;;
	std::cout << "\t\t Sender terminal: " << argv0 << " -r 0" << std::endl;
	std::cout << "\t\t Receiver terminal: " << argv0 << " -r 1" << std::endl;

	std::cout << "\t 2. For 2 machines: " << std::endl;
	std::cout << "\t\t Balanced PSI with best communication: " << std::endl;
	std::cout << "\t\t\t Sender terminal: " << argv0 << "-r 0 -n <log(setsize)> -t <#thread> -p 0 -ip <ip:port>" << std::endl;
	std::cout << "\t\t\t Receiver terminal: " << argv0 << "-r 1 -n <log(setsize)> -t <#thread> -p 0 -ip <ip:port>" << std::endl;
	std::cout << "\t\t\t Sender Example: " << argv0 << "-r 0 -n 16 -t 1 -p 0 -ip 172.31.22.179:1212" << std::endl;


	std::cout << "\t\t Balanced PSI with running time: " << std::endl;
	std::cout << "\t\t\t Sender terminal: " << argv0 << "-r 0 -n <log(setsize)> -t <#thread> -p 1 -ip <ip:port>" << std::endl;
	std::cout << "\t\t\t Receiver terminal: " << argv0 << "-r 1 -n <log(setsize)> -t <#thread> -p 1 -ip <ip:port>" << std::endl;
	std::cout << "\t\t\t Sender Example: " << argv0 << "-r 0 -n 16 -t 1 -p 1 -ip 172.31.22.179:1212" << std::endl;


	std::cout << "\t\t Unbalanced PSI: " << std::endl;
	std::cout << "\t\t\t Sender terminal: " << argv0 << "-r 0 -n <log(largesize)> -N <smallsize> -t <#thread> -ip <ip:port>" << std::endl;
	std::cout << "\t\t\t Receiver terminal: " << argv0 << "-r 1 -n <log(largesize)> -N <smallsize> -t <#thread> -ip <ip:port>" << std::endl;
	std::cout << "\t\t\t Sender Example: " << argv0 << "-r 0 -n 20 -N 5000 -t 1 -ip 172.31.22.179:1212" << std::endl;


}

//#define PRTY_C19
#ifdef PRTY_C19

void Sender(span<block> inputs, u64 theirSetSize, string ipAddr_Port, u64 numThreads = 1)
{
	u64 psiSecParam = 40;

	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));

	// set up networking
	std::string name = "n";
	IOService ios;
	Endpoint ep1(ios, ipAddr_Port, EpMode::Server, name);

	std::vector<Channel> sendChls(numThreads);
	for (u64 i = 0; i < numThreads; ++i)
		sendChls[i] = ep1.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));

	PrtySender sender;
	gTimer.reset();
	gTimer.setTimePoint("s_start");

	sender.init(inputs.size(), theirSetSize,40, prng0,sendChls);
	gTimer.setTimePoint("s_offline");
	
	if(inputs.size()!=theirSetSize && protocolId == 1) //unequal set size
		sender.outputBigPoly(inputs, sendChls);
	else
		if (protocolId == 0)
			sender.output(inputs, sendChls);
		else
			sender.outputBigPoly(inputs, sendChls);


	gTimer.setTimePoint("s_end");
	std::cout << gTimer << std::endl;

	for (u64 i = 0; i < numThreads; ++i)
		sendChls[i].close();

	ep1.stop();	ios.stop();
}


void Receiver( span<block> inputs, u64 theirSetSize, string ipAddr_Port, u64 numThreads=1)
{
	u64 psiSecParam = 40;

	PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));

	// set up networking
	std::string name = "n";
	IOService ios;
	Endpoint ep0(ios, ipAddr_Port, EpMode::Client, name);

	std::vector<Channel> sendChls(numThreads), recvChls(numThreads);
	for (u64 i = 0; i < numThreads; ++i)
		recvChls[i] = ep0.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));

	PrtyReceiver recv;
	gTimer.reset();
	gTimer.setTimePoint("r_start");

	recv.init(inputs.size(), theirSetSize,40, prng1,recvChls); //offline
	
	gTimer.setTimePoint("r_offline");
	
	if (inputs.size() != theirSetSize && protocolId == 1) //unequal set size
		recv.outputBigPoly(inputs, recvChls);
	else
		if (protocolId == 0)
			recv.output(inputs, recvChls);
		else
			recv.outputBigPoly(inputs, recvChls);


	
	gTimer.setTimePoint("r_end");

	std::cout << gTimer << std::endl;

	std::cout << "recv.mIntersection  : " << recv.mIntersection.size() << std::endl;
	std::cout << "expectedIntersection: " << expectedIntersection << std::endl;
	for (u64 i = 0; i < recv.mIntersection.size(); ++i)//thrds.size()
	{
		/*std::cout << "#id: " << recv.mIntersection[i] <<
			"\t" << inputs[recv.mIntersection[i]] << std::endl;*/
	}
	
	u64 dataSent = 0, dataRecv(0);
	for (u64 g = 0; g < recvChls.size(); ++g)
	{
		dataSent += recvChls[g].getTotalDataSent();
		dataRecv += recvChls[g].getTotalDataRecv();
		recvChls[g].resetStats();
	}

	std::cout << "      Total Comm = " << string_format("%5.2f", (dataRecv + dataSent) / std::pow(2.0, 20)) << " MB\n";
	
	for (u64 i = 0; i < numThreads; ++i)
		recvChls[i].close();

	ep0.stop(); ios.stop();

}

#include "Poly/polyFFT.h"
void Poly_Test_Impl() {


	int degree = 40;
	int numTrials = (degree - 1 + (1 << 8)) / degree;
	long numSlice = 4;
	int subField = 128;
	int fieldSize = 440;
	std::cout << "FieldSize: " << fieldSize << "\t";
	std::cout << "subField: " << subField << "\t";
	std::cout << "numSlice: " << numSlice << "\t";
	std::cout << "degree: " << degree << "\t";
	std::cout << "#bin: " << numTrials << "\n";


	//#################Quaratic GF2EX################################
	{
		NTL::GF2X mGf2x;
		NTL::BuildIrred(mGf2x, fieldSize);
		NTL::GF2E::init(mGf2x);
		NTL::GF2E e;

		NTL::vec_GF2E vecX, vecY;
		NTL::GF2EX polynomial;

		for (u64 i = 0; i <= degree; ++i)
		{
			NTL::random(e);
			vecX.append(e);
			NTL::random(e);
			vecY.append(e);
		}

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			polynomial = NTL::interpolate(vecX, vecY);

		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				e = NTL::eval(polynomial, vecX[i]); //2x
				e = NTL::eval(polynomial, vecX[i]); //2x
			}

		gTimer.setTimePoint("eval GF2EX n^2");
		std::cout << gTimer << std::endl;
	}



	//#################Slicing Quaratic GF2EX################################
	{
		NTL::GF2X gf2x;
		NTL::BuildIrred(gf2x, subField);
		NTL::GF2E::init(gf2x);
		NTL::GF2E e2;

		NTL::vec_GF2E vecX2;
		std::vector<NTL::vec_GF2E> vecY2(numSlice);
		NTL::GF2EX polynomial;

		for (u64 i = 0; i <= degree; ++i)
		{
			NTL::random(e2);
			vecX2.append(e2);
		}

		for (int j = 0; j < numSlice; j++)
		{
			for (unsigned int i = 0; i <= degree; i++)
			{
				NTL::random(e2);
				vecY2[j].append(e2);
			}
		}


		gTimer.reset();
		gTimer.setTimePoint("start");
		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
		{
			for (int j = 0; j < numSlice; j++)
				polynomial = NTL::interpolate(vecX2, vecY2[j]);
		}

		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				e2 = NTL::eval(polynomial, vecX2[i]); //2x
				e2 = NTL::eval(polynomial, vecX2[i]); //2x
			}

		gTimer.setTimePoint("eval slicing GF2EX n^2");


		std::cout << gTimer << std::endl;
	}

	//#################Quaratic ZZ_p################################
	{
		ZZ prime;
		GenGermainPrime(prime, fieldSize);
		ZZ_p::init(ZZ(prime));
		ZZ_p p;
		vec_ZZ_p x, y;

		NTL::ZZ_pX polynomial;

		for (u64 i = 0; i <= degree; ++i)
		{
			NTL::random(p);
			x.append(p);
			NTL::random(p);
			y.append(p);
		}

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			polynomial = NTL::interpolate(x, y);


		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				p = NTL::eval(polynomial, x[i]); //2x
				p = NTL::eval(polynomial, x[i]); //2x
			}

		gTimer.setTimePoint("eval Quaratic ZZ_p");
		std::cout << gTimer << std::endl;
	}

	//#################Quaratic ZZ_p Slicing################################
	{
		ZZ prime2;
		GenGermainPrime(prime2, subField);
		ZZ_p p;
		ZZ_p::init(ZZ(prime2));
		NTL::ZZ_pX polynomial;

		vec_ZZ_p x;
		for (unsigned int i = 0; i <= degree; i++) {
			NTL::random(p);
			x.append(p);
		}

		std::vector<vec_ZZ_p> y2(numSlice);

		for (int j = 0; j < numSlice; j++)
		{
			for (unsigned int i = 0; i <= degree; i++)
			{
				NTL::random(p);
				y2[j].append(p);
			}
		}

		gTimer.reset();
		gTimer.setTimePoint("start");
		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int j = 0; j < numSlice; j++)
				polynomial = NTL::interpolate(x, y2[j]);

		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				p = NTL::eval(polynomial, x[i]); //2x
				p = NTL::eval(polynomial, x[i]); //2x
			}

		gTimer.setTimePoint("eval slicing Quaratic ZZ_p");

		std::cout << gTimer << std::endl;
	}




	//#################nlog^2n Full################################
	{
		ZZ prime;
		GenGermainPrime(prime, fieldSize);

		// init underlying prime field
		ZZ_p::init(ZZ(prime));


		// interpolation points:
		ZZ_p* x = new ZZ_p[degree + 1];
		ZZ_p* y = new ZZ_p[degree + 1];
		for (unsigned int i = 0; i <= degree; i++) {
			random(x[i]);
			random(y[i]);
			//        cout << "(" << x[i] << "," << y[i] << ")" << endl;
		}

		ZZ_pX P;

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
		{
			interpolate_zp(P, x, y, degree, 1, prime);
		}
		gTimer.setTimePoint("interpolate nlog^2n ");
		std::cout << gTimer << std::endl;
	}

	//#################nlog^2n Slicing################################
	{
		ZZ prime2;
		GenGermainPrime(prime2, subField);
		ZZ_p::init(ZZ(prime2));


		ZZ_p* x2 = new ZZ_p[degree + 1];
		for (unsigned int i = 0; i <= degree; i++) {
			random(x2[i]);
		}

		std::vector<ZZ_p*> y2(4);
		for (int j = 0; j < 4; j++)
		{
			y2[j] = new ZZ_p[degree + 1];
			for (unsigned int i = 0; i <= degree; i++)
			{
				random(y2[j][i]);
			}
		}


		ZZ_pX *M = new ZZ_pX[degree * 2 + 1];;
		ZZ_p *a = new ZZ_p[degree + 1];;

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
		{
			prepareForInterpolate(x2, degree, M, a, 1, prime2);


			for (u64 j = 0; j < 4; j++)
			{
				ZZ_pX P;
				ZZ_pX* temp = new ZZ_pX[degree * 2 + 1];
				iterative_interpolate_zp(P, temp, y2[j], a, M, degree * 2 + 1, 1, prime2);
			}
		}
		gTimer.setTimePoint("interpolate slicing nlog^2n");

		std::cout << gTimer << std::endl;
	}


}

void Hashing_Test_Impl()
{
	setThreadName("Sender");
	u64 setSize = 1 << 20, psiSecParam = 40, numThreads(1);

	PRNG prng(_mm_set_epi32(4253465, 3434565, 234435, 23987045));


	std::vector<block> set(setSize);
	for (u64 i = 0; i < set.size(); ++i)
		set[i] = prng.get<block>();

	BalancedIndex simple;
	gTimer.reset();
	gTimer.setTimePoint("start");
	simple.init(setSize,40, 1);
	simple.insertItems(set);
	gTimer.setTimePoint("end");
	std::cout << gTimer << std::endl;
	simple.check();

//	simple.print(set);

}

void CuckooHasher_Test_Impl()
{
	u64 setSize = 1 << 20;

	u64 h = 2;
	std::vector<u64> _hashes(setSize * h + 1);
	MatrixView<u64> hashes(_hashes.begin(), _hashes.end(), h);
	PRNG prng(ZeroBlock);

	for (u64 i = 0; i < hashes.bounds()[0]; ++i)
	{
		for (u64 j = 0; j < h; ++j)
		{
			hashes[i][j] = prng.get<u64>();
		}
	}

	CuckooHasher hashMap0;
	CuckooHasher hashMap1;
	CuckooHasher::Workspace w(1);

	//hashMap0.init(setSize, 40, true);
	hashMap1.init(setSize, 40, true);

	for (u64 i = 0; i < setSize; ++i)
	{
		//if (i == 6) hashMap0.print();

		//hashMap0.insert(i, hashes[i]);

		std::vector<u64> tt{ i };
		MatrixView<u64> mm(hashes[i].data(), 1, 2);
		hashMap1.insertBatch(tt, mm, w);

		//if (i == 6) hashMap0.print();
		//if (i == 6) hashMap1.print();

		//if (hashMap0 != hashMap1)
		//{
		//    std::cout << i << std::endl;

		//    throw UnitTestFail();
		//}
	}

	hashMap1.print();


	/*if (hashMap0 != hashMap1)
	{
	throw UnitTestFail();
	}*/
}



void prfOtRow_Test_Impl()
{
	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));



	std::array<block, numSuperBlocks> rowQ;
	block x = prng0.get<block>();
	int setSize = 1 << 12;
	int binSize = 40;
	int numBin = (setSize + 39) / binSize;

	std::vector<AES> mAesQ(436);
	for (u64 i = 0; i < mAesQ.size(); i++)
	{
		x = prng0.get<block>();
		mAesQ[i].setKey(x);
	}

	std::vector<std::vector<std::array<block, numSuperBlocks>>> rowsQ1(numBin);
	std::vector<std::vector<std::array<block, numSuperBlocks>>> rowsQ2(numBin);
	std::vector<std::vector<block>> X(numBin);
	for (u64 i = 0; i < X.size(); i++)
	{
		X[i].resize(binSize);
		rowsQ1[i].resize(binSize);
		rowsQ2[i].resize(binSize);
		for (u64 j = 0; j < X[i].size(); j++)
			X[i][j] = prng0.get<block>();
	}


	gTimer.reset();
	gTimer.setTimePoint("start");
	for (u64 i = 0; i < numBin; i++)
	{
		for (u64 j = 0; j < binSize; j++)
			prfOtRow(X[i][j], rowsQ1[i][j], mAesQ);
	}
	gTimer.setTimePoint("prfOtRow end");
	std::cout << gTimer << std::endl;



	gTimer.reset();
	gTimer.setTimePoint("start");
	for (u64 i = 0; i < numBin; i++)
		prfOtRows(X[i], rowsQ2[i], mAesQ);
	gTimer.setTimePoint("numBin end");
	std::cout << gTimer << std::endl;

	for (u64 i = 0; i < numBin; i++)
	{
		for (u64 j = 0; j < binSize; j++)
			for (u64 k = 0; k < numSuperBlocks; j++)
				std::cout << rowsQ2[i][j][k] << "\t" << rowsQ1[i][j][k] << std::endl;

	}


	//rowsQ.resize(setSize);
	//X.resize(setSize);

	//for (u64 i = 0; i < X.size(); i++)
	//	X[i] = prng0.get<block>();

	//gTimer.reset();
	//gTimer.setTimePoint("start");
	//std::vector<block> ciphers(X.size());
	//mAesQ[0].ecbEncBlocks((block*)&X, X.size(), ciphers.data()); //do many aes at the same time for efficeincy

	////prfOtRows(X, rowsQ, mAesQ);
	//gTimer.setTimePoint("setSize end");


	std::cout << gTimer << std::endl;

}

void Prty_PSI_impl()
{
	setThreadName("Sender");
	u64 setSenderSize = 1 << 10, setRecvSize = 1 << 8, psiSecParam = 40, numThreads(2);

	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));


	std::vector<block> sendSet(setSenderSize), recvSet(setRecvSize);
	for (u64 i = 0; i < setSenderSize; ++i)
		sendSet[i] = prng0.get<block>();

	for (u64 i = 0; i < setRecvSize; ++i)
		recvSet[i] = prng0.get<block>();


	for (u64 i = 0; i < 10; ++i)
	{
		sendSet[i] = recvSet[i];
		//std::cout << "intersection: " <<sendSet[i] << "\n";
	}

	// set up networking
	std::string name = "n";
	IOService ios;
	Endpoint ep0(ios, "localhost", 1212, EpMode::Client, name);
	Endpoint ep1(ios, "localhost", 1212, EpMode::Server, name);

	std::vector<Channel> sendChls(numThreads), recvChls(numThreads);
	for (u64 i = 0; i < numThreads; ++i)
	{
		sendChls[i] = ep1.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
		recvChls[i] = ep0.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
	}


	PrtySender sender;
	PrtyReceiver recv;


	fillOneBlock(mOneBlocks);

	u64 binSize = 40;
	std::vector<block> X(binSize);
	std::vector<std::array<block, numSuperBlocks>> rowT(binSize);
	std::vector<std::array<block, numSuperBlocks>> rowU(binSize);
	std::vector<std::array<block, numSuperBlocks>> rowQ(binSize);
	for (u64 i = 0; i < X.size(); i++)
		X[i] = prng0.get<block>();

	auto thrd = std::thread([&]() {
		recv.init(recvSet.size(), sendSet.size(), 40, prng1, recvChls);
		recv.outputBigPoly(recvSet, recvChls);

		/*prfOtRows(X, rowT, recv.mAesT);
		prfOtRows(X, rowU, recv.mAesU);*/

		/*for (u64 i = 0; i < binSize; ++i)
		{
		prfOtRow(X[i], rowT[i], recv.mAesT);
		prfOtRow(X[i], rowU[i], recv.mAesU);
		}*/

	});

	sender.init(sendSet.size(), recvSet.size(), 40, prng0, sendChls);
	sender.outputBigPoly(sendSet, sendChls);
	//prfOtRows(X, rowQ, sender.mAesQ);

	/*for (u64 i = 0; i < binSize; ++i)
	prfOtRow(X[i], rowQ[i], sender.mAesQ);*/

	thrd.join();

	auto choiceBlocks = sender.mOtChoices.getSpan<block>(); //s

															//for (u64 i = 0; i < binSize; ++i)
	for (u64 j = 0; j < numSuperBlocks; ++j)
	{
		block rcvBlk = sender.subRowQForDebug[j] ^ ((recv.subRowTForDebug[j] ^ recv.subRowUForDebug[j]) & choiceBlocks[j]); //Q+s*P
		std::cout << "OT test: " << rcvBlk << "\t" << recv.subRowTForDebug[j] << std::endl;

		rcvBlk = recv.subRowTForDebug[j] ^ recv.subRowUForDebug[j];
		std::cout << "recv.rowR: " << rcvBlk << std::endl;

	}


	std::cout << "recv.mIntersection.size(): " << recv.mIntersection.size() << std::endl;
	for (u64 i = 0; i < recv.mIntersection.size(); ++i)//thrds.size()
	{
		std::cout << "#id: " << recv.mIntersection[i] <<
			"\t" << recvSet[recv.mIntersection[i]] << std::endl;
	}





	for (u64 i = 0; i < numThreads; ++i)

	{
		sendChls[i].close(); recvChls[i].close();
	}

	ep0.stop(); ep1.stop();	ios.stop();





}

#endif


#define PRTY2


#include "Poly/polyFFT.h"
void Poly_Test_Impl() {


	int degree = 1<<6;
	int numTrials = 1;// (degree - 1 + (1 << 8)) / degree;
	long numSlice = 4;
	int subField = 128;
	int fieldSize = 256;
	int numThread = 1;
	std::cout << "FieldSize: " << fieldSize << "\t";
	//std::cout << "subField: " << subField << "\t";
	//std::cout << "numSlice: " << numSlice << "\t";
	std::cout << "degree: " << degree << "\t";
	std::cout << "numThread: " << numThread << "\n";
	//std::cout << "#bin: " << numTrials << "\n";

#if 0

	//#################Quaratic GF2EX################################
	{
		NTL::GF2X mGf2x;
		NTL::BuildIrred(mGf2x, fieldSize);
		NTL::GF2E::init(mGf2x);
		NTL::GF2E e;

		NTL::vec_GF2E vecX, vecY;
		NTL::GF2EX polynomial;

		for (u64 i = 0; i <= degree; ++i)
		{
			NTL::random(e);
			vecX.append(e);
			NTL::random(e);
			vecY.append(e);
		}

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			polynomial = NTL::interpolate(vecX, vecY);

		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				e = NTL::eval(polynomial, vecX[i]); //2x
				e = NTL::eval(polynomial, vecX[i]); //2x
			}

		gTimer.setTimePoint("eval GF2EX n^2");
		std::cout << gTimer << std::endl;
	}



	//#################Slicing Quaratic GF2EX################################
	{
		NTL::GF2X gf2x;
		NTL::BuildIrred(gf2x, subField);
		NTL::GF2E::init(gf2x);
		NTL::GF2E e2;

		NTL::vec_GF2E vecX2;
		std::vector<NTL::vec_GF2E> vecY2(numSlice);
		NTL::GF2EX polynomial;

		for (u64 i = 0; i <= degree; ++i)
		{
			NTL::random(e2);
			vecX2.append(e2);
		}

		for (int j = 0; j < numSlice; j++)
		{
			for (unsigned int i = 0; i <= degree; i++)
			{
				NTL::random(e2);
				vecY2[j].append(e2);
			}
		}


		gTimer.reset();
		gTimer.setTimePoint("start");
		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
		{
			for (int j = 0; j < numSlice; j++)
				polynomial = NTL::interpolate(vecX2, vecY2[j]);
		}

		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				e2 = NTL::eval(polynomial, vecX2[i]); //2x
				e2 = NTL::eval(polynomial, vecX2[i]); //2x
			}

		gTimer.setTimePoint("eval slicing GF2EX n^2");


		std::cout << gTimer << std::endl;
	}

	//#################Quaratic ZZ_p################################
	{
		ZZ prime;
		GenGermainPrime(prime, fieldSize);
		ZZ_p::init(ZZ(prime));
		ZZ_p p;
		vec_ZZ_p x, y;

		NTL::ZZ_pX polynomial;

		for (u64 i = 0; i <= degree; ++i)
		{
			NTL::random(p);
			x.append(p);
			NTL::random(p);
			y.append(p);
		}

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			polynomial = NTL::interpolate(x, y);


		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				p = NTL::eval(polynomial, x[i]); //2x
				p = NTL::eval(polynomial, x[i]); //2x
			}

		gTimer.setTimePoint("eval Quaratic ZZ_p");
		std::cout << gTimer << std::endl;
	}


	//#################Quaratic ZZ_p Slicing################################
	{
		ZZ prime2;
		GenGermainPrime(prime2, subField);
		ZZ_p p;
		ZZ_p::init(ZZ(prime2));
		NTL::ZZ_pX polynomial;

		vec_ZZ_p x;
		for (unsigned int i = 0; i <= degree; i++) {
			NTL::random(p);
			x.append(p);
		}

		std::vector<vec_ZZ_p> y2(numSlice);

		for (int j = 0; j < numSlice; j++)
		{
			for (unsigned int i = 0; i <= degree; i++)
			{
				NTL::random(p);
				y2[j].append(p);
			}
		}

		gTimer.reset();
		gTimer.setTimePoint("start");
		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int j = 0; j < numSlice; j++)
				polynomial = NTL::interpolate(x, y2[j]);

		gTimer.setTimePoint("interpolate");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
			for (int i = 0; i <= degree; ++i)
			{
				p = NTL::eval(polynomial, x[i]); //2x
				p = NTL::eval(polynomial, x[i]); //2x
			}

		gTimer.setTimePoint("eval slicing Quaratic ZZ_p");

		std::cout << gTimer << std::endl;
	}


#endif

	//#################nlog^2n Full################################
	{
		ZZ prime;
		GenGermainPrime(prime, fieldSize);

		// init underlying prime field
		ZZ_p::init(ZZ(prime));


		// interpolation points:
		ZZ_p* x = new ZZ_p[degree + 1];
		ZZ_p* y = new ZZ_p[degree + 1];
		for (unsigned int i = 0; i <= degree; i++) {
			random(x[i]);
			random(y[i]);
			//        cout << "(" << x[i] << "," << y[i] << ")" << endl;
		}

		ZZ_pX P;

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
		{
			interpolate_zp(P, x, y, degree, numThread, prime);
		}
		gTimer.setTimePoint("interpolate nlog^2n ");
		std::cout << gTimer << std::endl;

		EllipticCurve curve(Curve25519, ZeroBlock);

		EccNumber a(curve);
		EccPoint xa(curve), point(curve);

		a.randomize(OneBlock);
		point.randomize(AllOneBlock);

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int i = 0; i < degree; ++i)
		{
			xa = (point * a);
		}
		gTimer.setTimePoint("exp done ");
		std::cout << gTimer << std::endl;


	}

#if 0
	//#################nlog^2n Slicing################################
	{
		ZZ prime2;
		GenGermainPrime(prime2, subField);
		ZZ_p::init(ZZ(prime2));


		ZZ_p* x2 = new ZZ_p[degree + 1];
		for (unsigned int i = 0; i <= degree; i++) {
			random(x2[i]);
		}

		std::vector<ZZ_p*> y2(4);
		for (int j = 0; j < 4; j++)
		{
			y2[j] = new ZZ_p[degree + 1];
			for (unsigned int i = 0; i <= degree; i++)
			{
				random(y2[j][i]);
			}
		}


		ZZ_pX* M = new ZZ_pX[degree * 2 + 1];;
		ZZ_p* a = new ZZ_p[degree + 1];;

		gTimer.reset();
		gTimer.setTimePoint("start");

		for (int iTrial = 0; iTrial < numTrials; ++iTrial)
		{
			prepareForInterpolate(x2, degree, M, a, 1, prime2);


			for (u64 j = 0; j < 4; j++)
			{
				ZZ_pX P;
				ZZ_pX* temp = new ZZ_pX[degree * 2 + 1];
				iterative_interpolate_zp(P, temp, y2[j], a, M, degree * 2 + 1, 1, prime2);
			}
		}
		gTimer.setTimePoint("interpolate slicing nlog^2n");

		std::cout << gTimer << std::endl;
	}
#endif

}


#ifdef PRTY2

void Prty2_Sender(span<block> inputs, u64 theirSetSize, bool isMalicious, string ipAddr_Port, u64 numThreads = 1)
{
	u64  psiSecParam = 40;

	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));

		// set up networking
	std::string name = "n";
	IOService ios;
	Endpoint ep1(ios, ipAddr_Port, EpMode::Server, name);

	std::vector<Channel> sendChls(numThreads);
	for (u64 i = 0; i < numThreads; ++i)
		sendChls[i] = ep1.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));

	PrtyMPsiSender sender;

	gTimer.reset();
	gTimer.setTimePoint("s_start");
	
	sender.init(inputs.size(), theirSetSize, 40, prng0, sendChls, isMalicious);
	gTimer.setTimePoint("s_offline");

	sender.output(inputs, sendChls);
	gTimer.setTimePoint("s_end");
	std::cout << gTimer << std::endl;

	for (u64 i = 0; i < numThreads; ++i)
		sendChls[i].close(); 

	 ep1.stop();	ios.stop();
}

void Prty2_Receiver(span<block> inputs, u64 theirSetSize, bool isMalicious, string ipAddr_Port, u64 numThreads = 1)
{
	u64  psiSecParam = 40;

	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));

	// set up networking
	std::string name = "n";
	IOService ios;
	Endpoint ep0(ios, ipAddr_Port, EpMode::Client, name);

	std::vector<Channel> recvChls(numThreads);
	for (u64 i = 0; i < numThreads; ++i)
		recvChls[i] = ep0.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));

	PrtyMPsiReceiver recv;

	gTimer.reset();
	gTimer.setTimePoint("r_start");

		recv.init(inputs.size(), theirSetSize, 40, prng1, recvChls, isMalicious);
		gTimer.setTimePoint("r_offline");

		recv.output(inputs, recvChls);
		gTimer.setTimePoint("r_end");

		std::cout << "\n==============Summary==============" << std::endl;
		std::cout << "SetSize: " << inputs.size() << " vs " << theirSetSize
			<< "   |  isMalicious: " << isMalicious << "\t";

		std::cout << gTimer << std::endl;


		u64 dataSent = 0, dataRecv(0);
		for (u64 g = 0; g < recvChls.size(); ++g)
		{
			dataSent += recvChls[g].getTotalDataSent();
			dataRecv += recvChls[g].getTotalDataRecv();
			recvChls[g].resetStats();
		}

		std::cout << "      Total Comm = " << string_format("%5.2f", (dataRecv + dataSent) / std::pow(2.0, 20)) << " MB\n";


	std::cout << "recv.mIntersection  : " << recv.mIntersection.size() << std::endl;
	std::cout << "expectedIntersection: " << expectedIntersection << std::endl;
	//for (u64 i = 0; i < recv.mIntersection.size(); ++i)//thrds.size()
	//{
	//	std::cout << "#id: " << recv.mIntersection[i] <<
	//		"\t" << inputs[recv.mIntersection[i]] << std::endl;
	//}


	for (u64 i = 0; i < numThreads; ++i)
	{
		recvChls[i].close();
	}

	ep0.stop();	ios.stop();

}

void Prty2_Psi_demo()
{
	u64 isMalicious = false;
	u64 setSenderSize = 1 << 8, setRecvSize = 1 << 8, psiSecParam = 40, numThreads(1);

	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));


	std::vector<block> sendSet(setSenderSize), recvSet(setRecvSize);
	for (u64 i = 0; i < setSenderSize; ++i)
		sendSet[i] = prng0.get<block>();

	for (u64 i = 0; i < setRecvSize; ++i)
		recvSet[i] = prng0.get<block>();


	for (u64 i = 0; i < 10; ++i)
	{
		sendSet[i] = recvSet[i];
		//std::cout << "intersection: " <<sendSet[i] << "\n";
	}

	// set up networking
	std::string name = "n";
	IOService ios;
	Endpoint ep0(ios, "localhost", 1212, EpMode::Client, name);
	Endpoint ep1(ios, "localhost", 1212, EpMode::Server, name);

	std::vector<Channel> sendChls(numThreads), recvChls(numThreads);
	for (u64 i = 0; i < numThreads; ++i)
	{
		sendChls[i] = ep1.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
		recvChls[i] = ep0.addChannel("chl" + std::to_string(i), "chl" + std::to_string(i));
	}

	PrtyMPsiSender sender;
	PrtyMPsiReceiver recv;

	auto thrd = std::thread([&]() {
		recv.init(recvSet.size(), sendSet.size(), 40, prng1, recvChls, isMalicious);
		recv.output(recvSet, recvChls);
		});

	sender.init(sendSet.size(), recvSet.size(), 40, prng0, sendChls, isMalicious);
	sender.output(sendSet, sendChls);

	thrd.join();



	std::cout << "recv.mIntersection.size(): " << recv.mIntersection.size() << std::endl;
	for (u64 i = 0; i < recv.mIntersection.size(); ++i)//thrds.size()
	{
		std::cout << "#id: " << recv.mIntersection[i] <<
			"\t" << recvSet[recv.mIntersection[i]] << std::endl;
	}


	for (u64 i = 0; i < numThreads; ++i)

	{
		sendChls[i].close(); recvChls[i].close();
	}

	ep0.stop(); ep1.stop();	ios.stop();


}


void curve_Test()
{



}
#endif

int main(int argc, char** argv)
{

	/*Poly_Test_Impl();
	return 0;*/

	/*Prty2_Psi_demo();
	return 0;*/
	//Prty2_Psi_demo();
	//return 0;

	/*CuckooHasher_Test_Impl();
	return 0;*/
	//#####################ECHD##############
	//curveType = 0 =>k286
	//./bin/frontend.exe -r 0 -echd -c 1 -n 8 & ./bin/frontend.exe -r 1 -echd -c 1 -n 8                                       


	string ipadrr = "localhost:1212";

	
	u64 sendSetSize = 1 << 12, recvSetSize = 1 << 12, numThreads = 1;
	bool isMalicious=true;
	

	if (argc == 9
		&& argv[3][0] == '-' && argv[3][1] == 'n'
		&& argv[5][0] == '-' && argv[5][1] == 'N'
		&& argv[9][0] == '-' && argv[9][1] == 'i' && argv[9][2] == 'p')
	{
		sendSetSize = 1 << atoi(argv[4]);
		recvSetSize =  atoi(argv[6]);
		ipadrr = argv[8];
	}

	if (argc == 9
		&& argv[3][0] == '-' && argv[3][1] == 'n'
		&& argv[5][0] == '-' && argv[5][1] == 's'
		&& argv[7][0] == '-' && argv[7][1] == 'i' && argv[7][2] == 'p')
	{
		sendSetSize = 1 << atoi(argv[4]);
		isMalicious = atoi(argv[6]);
		recvSetSize = sendSetSize;
		ipadrr = argv[6];
	}

	if (argc == 7
		&& argv[3][0] == '-' && argv[3][1] == 'n'
		&& argv[7][0] == '-' && argv[7][1] == 'i' && argv[7][2] == 'p')
	{
		sendSetSize = 1 << atoi(argv[4]);
		recvSetSize = sendSetSize;
		ipadrr = argv[6];
	}

	if (argc == 5
		&& argv[3][0] == '-' && argv[3][1] == 'n')
	{
		sendSetSize = 1 << atoi(argv[4]);
		recvSetSize = sendSetSize;
	}



		
	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	std::vector<block> sendSet(sendSetSize), recvSet(recvSetSize);
	
	std::cout << "SetSize: " << sendSetSize << " vs " << recvSetSize << "   |  isMalicious: " << isMalicious << "\n";
	
	
	for (u64 i = 0; i < sendSetSize; ++i)
		sendSet[i] = prng0.get<block>();

	for (u64 i = 0; i < recvSetSize; ++i)
		recvSet[i] = prng0.get<block>();

	for (u64 i = 0; i < expectedIntersection; ++i)
	{
		sendSet[i] = recvSet[i];
	}

	
#if 0
	std::thread thrd = std::thread([&]() {
		Prty2_Sender(sendSet, recvSetSize, isMalicious, "localhost:1212");

	});

	Prty2_Receiver(recvSet, sendSetSize, isMalicious, "localhost:1212");

	thrd.join();
	return 0;
#endif

	

	if (argv[1][0] == '-' && argv[1][1] == 't') {
		
		std::thread thrd = std::thread([&]() {
			Prty2_Sender(sendSet, recvSetSize, isMalicious,"localhost:1212", numThreads);
		});

		Prty2_Receiver(recvSet, sendSetSize, isMalicious, "localhost:1212", numThreads);

		thrd.join();

	}
	else if (argv[1][0] == '-' && argv[1][1] == 'r' && atoi(argv[2]) == 0) {
		Prty2_Sender(sendSet, recvSetSize, isMalicious, ipadrr, numThreads);
	}
	else if (argv[1][0] == '-' && argv[1][1] == 'r' && atoi(argv[2]) == 1) {
		Prty2_Receiver(recvSet, sendSetSize, isMalicious, ipadrr, numThreads);
	}
	else {
		usage(argv[0]);
	}


	
  
	return 0;
}
