#include "CuckooGraph_Tests.h"

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
#include "Poly/polyFFT.h"
#include "PsiDefines.h"

#include "PRTY2/PrtyMOtReceiver.h"
#include "PRTY2/PrtyMOtSender.h"
#include "Tools/BalancedIndex.h"
#include "Tools/SimpleIndex.h"
#include "Tools/CuckooHasher.h"
#include "Tools/CuckooGraph.h"
#include "Tools/GaussianElimination.h"

#include "Common.h"
#include <thread>
#include <vector>

#ifdef GetMessage
#undef GetMessage
#endif

#ifdef  _MSC_VER
#pragma warning(disable: 4800)
#endif //  _MSC_VER
#include "libOTe/Tools/bch511.h"


using namespace osuCrypto;

namespace tests_libOTe
{
	void Cuckoo_BackEdges_Circle_Test()
	{


		int m = 20;
		MyVisitor vis;
		ccGraph g(m); //init vertext 0,,m

		boost::add_edge(0, 1, g);
		boost::add_edge(1, 2, g);
		boost::add_edge(2, 0, g);  //triagnle 012  //3

		boost::add_edge(1, 4, g);
		boost::add_edge(4, 5, g);
		boost::add_edge(5, 6, g);
		boost::add_edge(6, 1, g); //1456  //4
		boost::add_edge(6, 2, g); //1456  //4

		boost::add_edge(3, 7, g); // 389 10 ... 3   //8
		boost::add_edge(7, 8, g);
		boost::add_edge(8, 3, g);
		boost::add_edge(3, 9, g);
		boost::add_edge(9, 10, g);
		//boost::add_edge(10, 3, g);

		boost::add_edge(15, 16, g);
		boost::add_edge(16, 17, g);

		boost::add_edge(18, 19, g);


		//boost::add_edge(17, 15, g);  //3


		boost::depth_first_search(g, boost::visitor(vis));

		auto dfs_circles = vis.GetDfsCircles();
		auto dfs_visitor = vis.GetDfsVisitor();

		std::cout << "=============\n";
		for (int i = 0; i < dfs_circles.size(); ++i)
		{

			if (dfs_circles[i].size() > 1) //circle
			{

				for (int k = 0; k < dfs_circles[i].size() - 1; ++k)
					std::cout << "tree_edge: " << dfs_circles[i][k] << '\n';

				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << "\n===\n\n";

			}
			else
				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << "\n===\n\n";

		}


		for (auto it = dfs_visitor.begin(); it != dfs_visitor.end(); ++it)
		{
			std::cout << "tree_edge: " << *it << "\n";

		}


	}

	void solveEquation_Test() {
		std::map<EdgeID, int> mEdgeIdxMap;
		ccGraph g(10); //init vertext 0,,m

		auto e1 = boost::add_edge(0, 1, g);
		mEdgeIdxMap.insert(pair<EdgeID, int>(e1.first, 1));

		auto e2 = boost::add_edge(1, 2, g);
		mEdgeIdxMap.insert(pair<EdgeID, int>(e2.first, 3));

		auto e3 = boost::add_edge(2, 0, g);  //triagnle 012  //3
		mEdgeIdxMap.insert(pair<EdgeID, int>(e3.first, 3));


		//	g.out_edge_list
		std::cout << mEdgeIdxMap[e3.first] << "\n";

	}


	void Cuckoo_BackEdges_Circle_with_Input_Test()
	{

		u64 setSize = 1 << 4;
		u64 mNumBins = 1.5 * setSize;
		std::cout << "input_size = " << setSize << "\n";
		std::cout << "bin_size = " << mNumBins << "\n";
		PRNG prng(ZeroBlock);


		std::vector<block> inputs(setSize);
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			inputs[idxItem] = prng.get<block>();

		MyVisitor graph;
		graph.init(setSize, 2, mNumBins * setSize, 80);
		graph.buidingGraph(inputs);
		std::cout << "graph.cuckooGraph.m_edges.size(): " << graph.mCuckooGraph.m_edges.size();


		boost::depth_first_search(graph.mCuckooGraph, boost::visitor(graph));

		auto dfs_circles = graph.GetDfsCircles();
		auto dfs_visitor = graph.GetDfsVisitor();

		std::cout << "=============\n";
		for (int i = 0; i < dfs_circles.size(); ++i)
		{

			if (dfs_circles[i].size() > 1) //circle
			{

				for (int k = 0; k < dfs_circles[i].size() - 1; ++k)
				{
					std::cout << "tree_edge: " << dfs_circles[i][k] << " == ";
					auto key = Edge2StringIncr(dfs_circles[i][k], graph.mNumBins);
					std::cout << graph.mEdgeIdxMap[key] << '\n';

				}
			}
			std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << " == ";
			auto key = Edge2StringIncr(dfs_circles[i][dfs_circles[i].size() - 1], graph.mNumBins);
			std::cout << graph.mEdgeIdxMap[key] << "\n===\n\n";
		}


		for (auto it = dfs_visitor.begin(); it != dfs_visitor.end(); ++it)
		{
			std::cout << "tree_edge: " << *it << " == ";
			auto key = Edge2StringIncr(*it, graph.mNumBins);
			std::cout << graph.mEdgeIdxMap[key] << "\n";

		}
		/*	for (int i = 0; i < dfs_connected_component.size(); ++i)
			{
				for (int j = 0; j < dfs_connected_component[i].size(); ++j)
				{


					for (int k = 0; k < dfs_connected_component[i][j].size(); ++k)
					{

					}

				}

			}*/

	}
	// Driver program 
	void Gaussian_Elimination_Test()
	{
		/* input matrix */

		std::vector < std::vector<bool>> mat(4);
		mat[0] = { 1,0,0,1,1 };
		mat[1] = { 1,0,1,1,0 };
		mat[2] = { 1,0,0,0,0 };
		mat[3] = { 0,0,0,0,1 };

		std::vector<block> Y{ _mm_set_epi64x(0, 4), _mm_set_epi64x(0, 4), _mm_set_epi64x(0, 3), _mm_set_epi64x(0, 2) };

		auto mat_check = mat;
		auto y_check = Y;

		auto x = gaussianElimination(mat, Y);

		for (int i = 0; i < mat_check.size(); i++)
		{
			block sum = ZeroBlock;
			for (u64 j = 0; j < mat_check[i].size(); j++)
			{
				if (mat_check[i][j])
					sum = sum ^ x[j];

			}
			if (neq(sum, y_check[i]))
			{
				std::cout << sum << " vs " << y_check[i] << "\n";
			}

		}

		//return 0;
	}

	void bitVector_Test() {
		PRNG prng(ZeroBlock);
		auto x = AllOneBlock ^ OneBlock;
		BitVector bit((u8*)& x, sizeof(block));
		std::cout << "bit = " << bit << "\n";

	}



	void Cuckoo_OKVS_Test()
	{

		u64 setSize = 1<<5;
		u64 numBin = 1.6 * setSize;
		u64 sigma = 40+40;
		std::cout << "input_size = " << setSize << "\n";
		std::cout << "bin_size = " << numBin << "\n";
		PRNG prng(ZeroBlock);


		std::vector<block> inputs(setSize), outputs(setSize);
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			inputs[idxItem] = prng.get<block>();

		MyVisitor graph;
		Cuckoo_encode(inputs, graph, numBin, sigma);
		Cuckoo_decode(outputs, graph);

		for (int i = 0; i < inputs.size(); ++i)
			if (neq(outputs[i], inputs[i]))
			{
				std::cout << i << ":" << outputs[i] << " decode vs " << inputs[i] << "\n";
				throw UnitTestFail();

			}

#if 0
		MyVisitor graph;
		graph.init(setSize, 2, numBin);
		graph.buidingGraph(inputs);
		std::cout << "\ngraph.cuckooGraph.m_edges.size(): " << graph.mCuckooGraph.m_edges.size();


		boost::depth_first_search(graph.mCuckooGraph, boost::visitor(graph));

		auto dfs_circles = graph.GetDfsCircles();
		auto dfs_visitor = graph.GetDfsVisitor();
		auto dfs_back_edge = graph.GetDfsBackEdge();

		std::cout << "\ndfs_circles.size(): " << dfs_circles.size();
		std::cout << "\ndfs_back_edge.size(): " << dfs_back_edge.size();
		std::cout << "\ndfs_visitor.size(): " << dfs_visitor.size();


		std::cout << "\n=============\n";
		for (int i = 0; i < dfs_circles.size(); ++i)
		{

			if (dfs_circles[i].size() > 1) //circle
			{

				for (int k = 0; k < dfs_circles[i].size() - 1; ++k)
				{
					std::cout << "tree_edge: " << dfs_circles[i][k] << " == ";
					auto key = Edge2StringIncr(dfs_circles[i][k], graph.mNumBins);
					std::cout << graph.mEdgeIdxMap[key] << '\n';

				}
			}
			std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << " == ";
			auto key = Edge2StringIncr(dfs_circles[i][dfs_circles[i].size() - 1], graph.mNumBins);
			std::cout << graph.mEdgeIdxMap[key] << "\n===\n\n";
		}


		for (auto it = dfs_visitor.begin(); it != dfs_visitor.end(); ++it)
		{
			std::cout << "tree_edge: " << *it << " == ";
			auto key = Edge2StringIncr(*it, graph.mNumBins);
			std::cout << graph.mEdgeIdxMap[key] << "\n";

		}

		//================Create linear equation
		AES mAesRfunction(prng.get<block>());
		std::vector<block> functionR(inputs.size());
		for (int i = 0; i < inputs.size(); i++)
		{
			functionR[i] = mAesRfunction.ecbEncBlock(inputs[i]);
		}

		int mSigma = 40 + 40;
		std::vector < std::vector<bool>> GaussMatrix, copy_GaussMatrix;
		std::vector<block> assocated_values, copy_assocated_values; //for test


		block equation;
		block assocated_value;

		for (int i = 0; i < dfs_circles.size(); ++i)
		{
			equation = ZeroBlock;
			assocated_value = ZeroBlock;
			for (int j = 0; j < dfs_circles[i].size(); ++j) { // each circle

				auto keyEgdeMapping = Edge2StringIncr(dfs_circles[i][j], graph.mNumBins);
				auto idx_item_in_circle = graph.mEdgeIdxMap[keyEgdeMapping];
				equation = equation ^ functionR[idx_item_in_circle];
				assocated_value = assocated_value ^ inputs[idx_item_in_circle];

				BitVector coeff((u8*)& equation, mSigma);
				auto row = bitVector2BoolVector(coeff);
				//print_BoolVector(row);
				//std::cout << coeff << "\n";
				GaussMatrix.push_back(row);
				assocated_values.push_back(assocated_value);
			}
		}
		copy_GaussMatrix = GaussMatrix;//for test
		copy_assocated_values = assocated_values;

		//Solution 
		std::vector<block> R(mSigma, ZeroBlock);

		if (GaussMatrix.size() > 0)
			R = gaussianElimination(GaussMatrix, assocated_values);

		/*for (size_t i = 0; i < copy_assocated_values.size(); i++)
		{
			std::cout << copy_assocated_values[i] << " ==copy_assocated_values== " << assocated_values[i] << "\n";

		}*/

		for (int i = 0; i < copy_GaussMatrix.size(); i++)
		{
			block sum = ZeroBlock;
			for (u64 j = 0; j < copy_GaussMatrix[i].size(); j++)
			{
				if (copy_GaussMatrix[i][j])
					sum = sum ^ R[j];
			}

			if (neq(sum, copy_assocated_values[i]))
			{
				std::cout << sum << " sum vs " << copy_assocated_values[i] << "\n";
				std::cout << "failed " << i << std::endl;
				throw UnitTestFail();
			}

		}

		//================Fill D
		std::vector<block> L(graph.mNumBins, AllOneBlock);

		bool isRoot;
		for (auto it = dfs_visitor.begin(); it != dfs_visitor.end(); ++it)
		{
			auto edge = *it;
			auto keyEgdeMapping = Edge2StringIncr(edge, graph.mNumBins);
			auto idxItem = graph.mEdgeIdxMap[keyEgdeMapping];
			
			bool isRoot = eq(L[edge.m_source], AllOneBlock);
			if (isRoot) //root
			{
				L[edge.m_source] =  prng.get<block>();
				std::cout <<idxItem << " , ";

			}
			
			

			//compute h2
			if (eq(L[edge.m_target], AllOneBlock))
			{
				L[edge.m_target] = L[edge.m_source] ^ inputs[idxItem];
				block valueR = functionR[idxItem];
				BitVector coeff((u8*)& valueR, mSigma);
				for (int b = 0; b < coeff.size(); b++)
				{
					if (coeff[b])
						L[edge.m_target] = L[edge.m_target] ^ R[b];
				}
			}
			else {
				std::cout << " \n " << idxItem << " idx ";

				cout << edge.m_target << " L: " << L[edge.m_target] << " already fixed \n";

			}
		}

		//==========decode
		for (int i = 0; i < inputs.size(); ++i)
		{
			auto h1 = graph.hashes1[i];
			auto h2 = graph.hashes2[i];
			auto x = L[h1] ^ L[h2];

			block valueR = functionR[i];
			BitVector coeff((u8*)& valueR, mSigma);
			//std::cout << coeff << "\n";
			for (int b = 0; b < coeff.size(); b++)
			{
				if (coeff[b])
				{
					//std::cout << coeff[b] << " coeff[b]\n";
					x = x ^ R[b];
				}
			}
			if (neq(x, inputs[i]))
			{
				std::cout << i << ":" << x << " decode vs " << inputs[i] << "\n";
			}

		}


#endif
	}

	void testNco(
		NcoOtExtSender& sender,
		const u64& numOTs,
		PRNG& prng0,
		Channel& sendChl,
		NcoOtExtReceiver& recv,
		PRNG& prng1,
		Channel& recvChl)
	{

		u64 stepSize = 33;
		std::vector<block> inputs(stepSize);
		setThreadName("Receiver");

		for (size_t j = 0; j < 10; j++)
		{
			// perform the init on each of the classes. should be performed concurrently
			auto thrd = std::thread([&]() {
				setThreadName("Sender");
				sender.init(numOTs, prng0, sendChl);
				});
			recv.init(numOTs, prng1, recvChl);
			thrd.join();

			std::vector<block> encoding1(stepSize), encoding2(stepSize);

			// Get the random OT messages
			for (u64 i = 0; i < numOTs; i += stepSize)
			{
				auto curStepSize = std::min<u64>(stepSize, numOTs - i);
				std::vector<u8> skips(curStepSize);

				prng0.get(inputs.data(), inputs.size());
				prng0.get((bool*)skips.data(), skips.size());

				for (u64 k = 0; k < curStepSize; ++k)
				{

					// The receiver MUST encode before the sender. Here we are only calling encode(...) 
					// for a single i. But the receiver can also encode many i, but should only make one 
					// call to encode for any given value of i.
					if (skips[k])
					{
						recv.zeroEncode(i + k);
					}
					else {
						recv.encode(i + k, &inputs[k], (u8*)& encoding1[k], sizeof(block));
					}
				}

				// This call will send to the other party the next "curStepSize " corrections to the sender.
				// If we had made more or less calls to encode above (for contigious i), then we should replace
				// curStepSize  with however many calls we made. In an extreme case, the reciever can perform
				// encode for i \in {0, ..., numOTs - 1}  and then call sendCorrection(recvChl, numOTs).
				recv.sendCorrection(recvChl, curStepSize);

				// receive the next curStepSize  correction values. This allows the sender to now call encode
				// on the next curStepSize  OTs.
				sender.recvCorrection(sendChl, curStepSize);

				for (u64 k = 0; k < curStepSize; ++k)
				{
					// the sender can now call encode(i, ...) for k \in {0, ..., i}. 
					// Lets encode the same input and then we should expect to
					// get the same encoding.
					sender.encode(i + k, &inputs[k], (u8*)& encoding2[k], sizeof(block));

					// check that we do in fact get the same value
					if (!skips[k] && neq(encoding1[k], encoding2[k]))
						throw UnitTestFail("ot[" + ToString(i + k) + "] not equal " LOCATION);

					// In addition to the sender being able to obtain the same value as the receiver,
					// the sender can encode and other codeword. This should result in a different 
					// encoding.
					inputs[k] = prng0.get<block>();

					sender.encode(i + k, &inputs[k], (u8*)& encoding2[k], sizeof(block));

					if (eq(encoding1[k], encoding2[k]))
						throw UnitTestFail(LOCATION);
				}
			}
		}


		//block input;
		//for (size_t j = 0; j < 2; j++)
		//{

		//    auto thrd = std::thread([&]() {
		//        sender.init(numOTs, prng0, sendChl);
		//    });

		//    recv.init(numOTs, prng1, recvChl);

		//    thrd.join();


		//    for (u64 i = 0; i < numOTs; ++i)
		//    {
		//        block input = prng0.get<block>();


		//        bool skip = prng0.get<bool>();

		//        block encoding1, encoding2;
		//        if (skip)
		//        {
		//            recv.zeroEncode(i);
		//        }
		//        else
		//        {
		//            recv.encode(i, &input, &encoding1);
		//        }

		//        recv.sendCorrection(recvChl, 1);
		//        sender.recvCorrection(sendChl, 1);

		//        sender.encode(i, &input, &encoding2);

		//        if (!skip && neq(encoding1, encoding2))
		//        {
		//            std::cout << " = failed " << i << std::endl;
		//            throw UnitTestFail(LOCATION);
		//        }

		//        input = prng0.get<block>();


		//        sender.encode(i, &input, &encoding2);

		//        if (!skip && eq(encoding1, encoding2))
		//        {
		//            std::cout << " != failed " << i << std::endl;
		//            throw UnitTestFail(LOCATION);
		//        }
		//    }

		//    thrd = std::thread([&]() {recv.check(recvChl, ZeroBlock); });
		//    try {

		//        sender.check(sendChl, ZeroBlock);
		//    }
		//    catch (...)
		//    {
		//        std::cout << " check failed " << std::endl;
		//        thrd.join();
		//        throw;
		//    }

		//    thrd.join();
		//}
	}


	void PrtyMOt_Test_Impl()
	{
		setThreadName("Sender");

		PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
		PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));

		u64 numOTs = 128 * 2;


		std::string name = "n";
		IOService ios(0);
		Session ep0(ios, "localhost", 1212, SessionMode::Server, name);
		Session ep1(ios, "localhost", 1212, SessionMode::Client, name);
		auto recvChl = ep1.addChannel(name, name);
		auto sendChl = ep0.addChannel(name, name);


		LinearCode code;
		code.load(bch511_binary, sizeof(bch511_binary));

		PrtyMOtSender sender;
		PrtyMOtReceiver recv;

		sender.configure(true, 40, 50);
		recv.configure(true, 40, 50);
		u64 baseCount = sender.getBaseOTCount();
		//u64 codeSize = (baseCount + 127) / 128;

		std::vector<block> baseRecv(baseCount);
		std::vector<std::array<block, 2>> baseSend(baseCount);
		BitVector baseChoice(baseCount);
		baseChoice.randomize(prng0);

		prng0.get((u8*)baseSend.data()->data(), sizeof(block) * 2 * baseSend.size());
		for (u64 i = 0; i < baseCount; ++i)
		{
			baseRecv[i] = baseSend[i][baseChoice[i]];
		}

		sender.setBaseOts(baseRecv, baseChoice);
		recv.setBaseOts(baseSend);

		testNco(sender, numOTs, prng0, sendChl, recv, prng1, recvChl);

		auto v = std::async([&] { recv.check(recvChl, ZeroBlock); });
		try {
			sender.check(sendChl, ZeroBlock);

		}
		catch (...)
		{
			sendChl.mBase->mLog;
		}
		v.get();

		auto sender2 = sender.split();
		auto recv2 = recv.split();

		testNco(*sender2, numOTs, prng0, sendChl, *recv2, prng1, recvChl);

	}


}