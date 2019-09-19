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
#include "Tools/mx_132_by_583.h"


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

		u64 setSize = 1 << 9;
		u64 numBin = 1.8 * setSize;
		u64 sigma = 42;

		std::cout << "input_size = " << setSize << "\n";
		std::cout << "bin_size = " << numBin << "\n";
		PRNG prng(ZeroBlock);


		std::vector<block> inputs(setSize);
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			inputs[idxItem] = prng.get<block>();

		MyVisitor graph;
		graph.init(setSize, 2, numBin, sigma);
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

		u64 setSize = 1<<9;
		u64 numBin = 3 * setSize;
		u64 sigma = 80;
		std::cout << "input_size = " << setSize << "\n";
		std::cout << "bin_size = " << numBin << "\n";
		PRNG prng(ZeroBlock);


		std::vector<block> inputs(setSize), outputs(setSize), L, R;
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			inputs[idxItem] = prng.get<block>();

		MyVisitor graph;
		Cuckoo_encode(inputs, L,numBin, sigma);
		Cuckoo_decode(outputs, inputs, L, numBin);

		for (int i = 0; i < inputs.size(); ++i)
			if (neq(outputs[i], inputs[i]))
			{
				std::cout << i << ":" << outputs[i] << " decodedecode vs " << inputs[i] << "\n";
				//throw UnitTestFail();

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

	void PrtyLinearCode_Test_Impl()
	{
		PRNG prng(ZeroBlock);
		LinearCode code;
		code.load(mx132by583, sizeof(mx132by583));

		u64 plain_txt_bits = code.plaintextBitSize();
		if (plain_txt_bits != 132) {
			cout << "Invalid plain text bit-size " << plain_txt_bits << endl;
			throw UnitTestFail("bad out size reported by code");
		}

		u64 plain_txt_blocks = code.plaintextBlkSize();
		if (plain_txt_blocks != 2) {
			cout << "Invalid plain text block-size " << plain_txt_bits << endl;
			throw UnitTestFail("bad out size reported by code");
		}

		u64 code_word_bits = code.codewordBitSize();
		if (code_word_bits != 583) {
			cout << "Invalid code word bit-size " << code_word_bits << endl;
			throw UnitTestFail("bad out size reported by code");
		}

		u64 code_word_blocks = code.codewordBlkSize();
		if (code_word_blocks != 5) {
			cout << "Invalid code word block-size " << code_word_bits << endl;
			throw UnitTestFail("bad out size reported by code");
		}


		for (size_t testk = 0; testk < 2; ++testk) {
			cout << "lctest begin" << endl;

			std::vector<block> plainTextA(plain_txt_blocks), codewordA(code_word_blocks);
			std::vector<block> plainTextB(plain_txt_blocks), codewordB(code_word_blocks);
			std::vector<block> plainTextC(plain_txt_blocks), codewordC(code_word_blocks);

			for (size_t i = 0; i < plain_txt_blocks; ++i) {
				plainTextA[i] = prng.get<block>();
				plainTextB[i] = prng.get<block>();
				plainTextC[i] = plainTextA[i] ^ plainTextB[i];
			}

		/*	zero_block_ls_bits(plainTextA[plain_txt_blocks - 1], (8 * sizeof(osuCrypto::block) * plain_txt_blocks) % plain_txt_bits);
			zero_block_ls_bits(plainTextB[plain_txt_blocks - 1], (8 * sizeof(osuCrypto::block) * plain_txt_blocks) % plain_txt_bits);
			zero_block_ls_bits(plainTextC[plain_txt_blocks - 1], (8 * sizeof(osuCrypto::block) * plain_txt_blocks) % plain_txt_bits);
*/
			code.encode(plainTextA, codewordA);
			code.encode(plainTextB, codewordB);
			code.encode(plainTextC, codewordC);

			/*zero_block_ls_bits(codewordA[code_word_blocks - 1], (8 * sizeof(osuCrypto::block) * code_word_blocks) % code_word_bits);
			zero_block_ls_bits(codewordB[code_word_blocks - 1], (8 * sizeof(osuCrypto::block) * code_word_blocks) % code_word_bits);
			zero_block_ls_bits(codewordC[code_word_blocks - 1], (8 * sizeof(osuCrypto::block) * code_word_blocks) % code_word_bits);
*/
			bool mismatch = false;
			for (size_t i = 0; i < code_word_blocks; ++i) {

				if (!eq(codewordA[i] ^ codewordB[i], codewordC[i])) {
					cout << "Test " << testk << " linear mismatch @" << i << endl;
					mismatch = true;
				}
			}
			if (!mismatch) {
				cout << "Test " << testk << " linear match." << endl;
			}
			else {
				cout << "A:" << endl;
				for (size_t i = 0; i < code_word_blocks; ++i) {
					cout << codewordA[i] << endl;
	}
				cout << "B:" << endl;
				for (size_t i = 0; i < code_word_blocks; ++i) {
					cout << codewordB[i] << endl;
				}
				cout << "C:" << endl;
				for (size_t i = 0; i < code_word_blocks; ++i) {
					cout << codewordC[i] << endl;
				}
		}
	}



#if 0 //BCH
		code.load(bch511_binary, sizeof(bch511_binary));

		if (code.plaintextBitSize() != 76)
			throw UnitTestFail("bad input size reported by code");


		if (code.codewordBitSize() != 511)
			throw UnitTestFail("bad out size reported by code");

		std::vector<block>
			plainText(code.plaintextBlkSize(), AllOneBlock),
			codeword(code.codewordBlkSize());
		//gsl::span<u8>ss(plainText);
		code.encode(plainText, codeword);

		BitVector cw((u8*)codeword.data(), code.codewordBitSize());

		// expect all ones
		for (size_t i = 0; i < cw.size(); i++)
		{
			if (cw[i] == 0)
			{
				std::cout << cw << std::endl;
				std::cout << "expecting all ones" << std::endl;
				throw UnitTestFail(LOCATION);
			}
		}

		BitVector pt("1111111111111111111111111111111111111111111111111101111111111101111111111111");
		memset(plainText.data(), 0, plainText.size() * sizeof(block));
		memcpy(plainText.data(), pt.data(), pt.sizeBytes());


		code.encode(plainText, codeword);
		cw.resize(0);
		cw.append((u8*)codeword.data(), code.codewordBitSize());


		BitVector expected("1111111111111111111111111111111111111111111111111101111111111101111111111111101000010001110100011100010110011111110010011010001010000111111001101101110101100000100010010101000110011001111101111100100111000101110000101000000011000100011110011100001101100111111001001011010100010010110001010011000011111010101010010010011101001001100001100010100101001100111000010110011110011110001110001011111101010001101000101010110100011000000011010011110101011001100011111111101001101111001111111101000010000011010111100011100");

		if (cw != expected)
		{
			std::cout << cw << std::endl;
			std::cout << expected << std::endl;
			throw UnitTestFail(LOCATION);
		}


		code.encode_bch511((u8*)plainText.data(), (u8*)codeword.data());
		cw.resize(0);
		cw.append((u8*)codeword.data(), code.codewordBitSize());


		expected = BitVector("1111111111111111111111111111111111111111111111111101111111111101111111111111101000010001110100011100010110011111110010011010001010000111111001101101110101100000100010010101000110011001111101111100100111000101110000101000000011000100011110011100001101100111111001001011010100010010110001010011000011111010101010010010011101001001100001100010100101001100111000010110011110011110001110001011111101010001101000101010110100011000000011010011110101011001100011111111101001101111001111111101000010000011010111100011100");

		if (cw != expected)
		{
			std::cout << cw << std::endl;
			std::cout << expected << std::endl;
			throw UnitTestFail(LOCATION);
		}


#endif

	}



	void PrtyMOt_Test_Impl()
	{
		setThreadName("Sender");

		PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
		PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));

		u64 numOTs = 128 * 2;
		std::vector<block> inputs(numOTs);
		prng0.get(inputs.data(), inputs.size());



		std::string name = "n";
		IOService ios(0);
		Session ep0(ios, "localhost", 1212, SessionMode::Server, name);
		Session ep1(ios, "localhost", 1212, SessionMode::Client, name);
		auto recvChl = ep1.addChannel(name, name);
		auto sendChl = ep0.addChannel(name, name);


		LinearCode code;
		code.load(mx132by583, sizeof(mx132by583));

		PrtyMOtSender sender;
		PrtyMOtReceiver recv;

		sender.configure(true, 40, 132);
		recv.configure(true, 40, 132);
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

			for (u64 k = 0; k < curStepSize; ++k)
			{

				// The receiver MUST encode before the sender. Here we are only calling encode(...) 
				// for a single i. But the receiver can also encode many i, but should only make one 
				// call to encode for any given value of i.
				recv.encode(i + k, &inputs[k], (u8*)& encoding1[k], sizeof(block));
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

				std::cout << encoding1[k] << " vs " << encoding2[k] << "\n";
				// check that we do in fact get the same value
				if (neq(encoding1[k], encoding2[k]))
					throw UnitTestFail("ot[" + ToString(i + k) + "] not equal " LOCATION);

			}
		}


	}


	void Prty2PSI_Test_Impl()
	{
		
#if 1
		setThreadName("Sender");

		PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
		PRNG prng1(_mm_set_epi32(4253465, 3434565, 234435, 23987025));

		u64 setSize = 1 << 6;
		u64 numBin = 3 * setSize;
		u64 sigma = 80;
		std::cout << "input_size = " << setSize << "\n";
		std::cout << "bin_size = " << numBin << "\n";
		std::vector<block> inputs(setSize), outputs(setSize), cuckooTables;
		prng0.get(inputs.data(), inputs.size());

		Cuckoo_encode(inputs, cuckooTables, numBin, sigma);  //Building CUckoo table
		//Cuckoo_decode(outputs, inputs, L, R);

		u64 numOTs = cuckooTables.size();
		std::cout << "OT = " << numOTs << "\n";


		std::string name = "n";
		IOService ios(0);
		Session ep0(ios, "localhost", 1212, SessionMode::Server, name);
		Session ep1(ios, "localhost", 1212, SessionMode::Client, name);
		auto recvChl = ep1.addChannel(name, name);
		auto sendChl = ep0.addChannel(name, name);


		LinearCode code;
		code.load(mx132by583, sizeof(mx132by583));

		PrtyMOtSender sender;
		PrtyMOtReceiver recv;

		sender.configure(true, 40, 132);
		recv.configure(true, 40, 132);
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

		// perform the init on each of the classes. should be performed concurrently
		auto thrd = std::thread([&]() {
			setThreadName("Sender");
			sender.init(numOTs, prng0, sendChl);
			});

		recv.init(numOTs, prng1, recvChl);
		thrd.join();

		
		std::vector<block> encoding1(numOTs), encoding2(numOTs);

		// Get the random OT messages
		for (u64 i = 0; i < numOTs; i += stepSize)
		{
			auto curStepSize = std::min<u64>(stepSize, numOTs - i);

			for (u64 k = 0; k < curStepSize; ++k)
			{

				// The receiver MUST encode before the sender. Here we are only calling encode(...) 
				// for a single i. But the receiver can also encode many i, but should only make one 
				// call to encode for any given value of i.
				recv.encode(i + k, &cuckooTables[k+i], (u8*)& encoding1[k+i], sizeof(block));
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
				sender.encode(i + k, &cuckooTables[k+i], (u8*)& encoding2[i+k], sizeof(block));

			//	std::cout << encoding1[k+i] << " vs " << encoding2[k+i] << "\n";
				// check that we do in fact get the same value
				if (neq(encoding1[i+k], encoding2[k+i]))
					throw UnitTestFail("ot[" + ToString(i + k) + "] not equal " LOCATION);

			}
		}

#endif

	}



}