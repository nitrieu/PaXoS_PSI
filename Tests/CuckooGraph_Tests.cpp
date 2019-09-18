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

#include "PRTY/PrtySender.h"
#include "PRTY/PrtyReceiver.h"
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

}