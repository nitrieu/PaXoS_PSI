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


		boost::add_edge(15, 16, g);
		boost::add_edge(16, 17, g);

		boost::add_edge(18, 19, g);


		//boost::add_edge(17, 15, g);  //3

		MyVisitor vis;
		boost::depth_first_search(g, boost::visitor(vis));

		auto dfs_circles = vis.GetDfsCircles();
		auto dfs_non_circles = vis.GetDfsNOCircles();

		std::cout << "dfs_circles.size(): " << dfs_circles.size() << "\n";
		std::cout << "dfs_non_circles.size(): " << dfs_non_circles.size() <<"\n";

		for (int i = 0; i < dfs_circles.size(); ++i)
		{
			if (dfs_circles[i].size() > 1) //circle
			{
				for (int j = 0; j < dfs_circles[i].size() - 1; ++j)
					std::cout << "tree_edge: " << dfs_circles[i][j] << '\n';

				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << "\n\n";

			}
			else 
				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << "\n\n";

		}

		for (int i = 0; i < dfs_non_circles.size(); ++i)
		{
			for (int j = 0; j < dfs_non_circles[i].size(); ++j)
				std::cout << "tree_edge: " << dfs_non_circles[i][j] << "\n";

			std::cout << "\n";

		}


	}
	void Cuckoo_BackEdges_Circle_with_Input_Test()
	{

		u64 setSize = 1 << 10;
		u64 mNumBins = 1.5 * setSize;
		std::cout << "input_size = " << setSize << "\n";
		std::cout << "bin_size = " << mNumBins << "\n";
		PRNG prng(ZeroBlock);


		std::vector<block> inputs(setSize);
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			inputs[idxItem] = prng.get<block>();

		CuckooGraph graph;
		graph.init(setSize, 2, 2.4 * setSize);
		graph.buidingGraph(inputs);
		std::cout << "graph.cuckooGraph.m_edges.size(): " << graph.mCuckooGraph.m_edges.size();


		MyVisitor vis;
		boost::depth_first_search(graph.mCuckooGraph, boost::visitor(vis));

		auto dfs_circles = vis.GetDfsCircles();
		auto dfs_non_circles = vis.GetDfsNOCircles();

		std::cout << "dfs_circles.size(): " << dfs_circles.size() << "\n";
		std::cout << "dfs_non_circles.size(): " << dfs_non_circles.size() << "\n";

		for (int i = 0; i < dfs_circles.size(); ++i)
		{
			if (dfs_circles[i].size() > 1) //circle
			{
				for (int j = 0; j < dfs_circles[i].size() - 1; ++j)
					std::cout << "tree_edge: " << dfs_circles[i][j] << '\n';

				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << "\n\n";

			}
			else
				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << "\n";

			std::cout << "===========================\n";

		}

		for (int i = 0; i < dfs_non_circles.size(); ++i)
		{
			for (int j = 0; j < dfs_non_circles[i].size(); ++j)
				std::cout << "tree_edge: " << dfs_non_circles[i][j] << "\n";

			std::cout << "===========================\n";


		}

		//std::cout << dfs_tree_edges.size() << "  dfs_tree_edges.size()  " << dfs_cnt_tree_edge << '\n';
		//std::cout << g.m_edges.size() << "  g.m_edges.size \n";

		//mDfs_tree_edges.clear();
		//mDfs_cnt_tree_edge = 0;
		//MyVisitor vis;
		//boost::depth_first_search(graph.cuckooGraph, boost::visitor(vis));


		//for (int i = 0; i < mDfs_tree_edges.size(); ++i)
		//{
		//	//	std::cout << dfs_tree_edges[i].first << " == " << dfs_tree_edges[i].second << '\n';
		//}

		//std::cout << mDfs_tree_edges.size() << "  dfs_tree_edges.size()  " << mDfs_cnt_tree_edge << '\n';



	}

}