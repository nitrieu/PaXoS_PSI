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
					std::cout << "tree_edge: " << dfs_circles[i][j]  <<   '\n';

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

	void solveEquation_Test() {
		std::map<EdgeID, int> mEdgeIdxMap;
		ccGraph g(10); //init vertext 0,,m
		
		auto e1= boost::add_edge(0, 1, g);
		mEdgeIdxMap.insert(pair<EdgeID, int>(e1.first, 1));

		auto e2= boost::add_edge(1, 2, g);
		mEdgeIdxMap.insert(pair<EdgeID, int>(e2.first, 3));

		auto e3 =  boost::add_edge(2, 0, g);  //triagnle 012  //3
		mEdgeIdxMap.insert(pair<EdgeID, int>(e3.first, 3));


	//	g.out_edge_list
		std::cout << mEdgeIdxMap[e3.first] << "\n";

	}


	void Cuckoo_BackEdges_Circle_with_Input_Test()
	{

		u64 setSize = 1 <<4;
		u64 mNumBins = 1.5 * setSize;
		std::cout << "input_size = " << setSize << "\n";
		std::cout << "bin_size = " << mNumBins << "\n";
		PRNG prng(ZeroBlock);


		std::vector<block> inputs(setSize);
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
			inputs[idxItem] = prng.get<block>();

		CuckooGraph graph;
		graph.init(setSize, 2,1.5* setSize);
		graph.buidingGraph(inputs);
		std::cout << "graph.cuckooGraph.m_edges.size(): " << graph.mCuckooGraph.m_edges.size();

	/*	for (auto elem = graph.mEdgeIdxMap.begin(); elem != graph.mEdgeIdxMap.end(); ++elem)
		{
			std::cout << elem->first << " ==== " << elem->second << "\n";

		}
*/

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
				{
					std::cout << "tree_edge: " << dfs_circles[i][j] << " - ";
				
					auto key = Edge2StringIncr(dfs_circles[i][j], graph.mNumBins);
					std::cout << graph.mEdgeIdxMap[key] << '\n';


				}
				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << " - ";
				auto key = Edge2StringIncr(dfs_circles[i][dfs_circles[i].size() - 1], graph.mNumBins);
				std::cout << graph.mEdgeIdxMap[key] << '\n';

			}
			else
			{
				std::cout << "back_edge: " << dfs_circles[i][dfs_circles[i].size() - 1] << " - ";
				auto key = Edge2StringIncr(dfs_circles[i][dfs_circles[i].size() - 1], graph.mNumBins);
				std::cout << graph.mEdgeIdxMap[key] << '\n';
			}

			std::cout << "===========================\n";

		}

		for (int i = 0; i < dfs_non_circles.size(); ++i)
		{
			for (int j = 0; j < dfs_non_circles[i].size(); ++j)
			{
				std::cout << "tree_edge: " << dfs_non_circles[i][j] << " - ";

				auto key = Edge2StringIncr(dfs_non_circles[i][j], graph.mNumBins);
				std::cout << graph.mEdgeIdxMap[key] << '\n';
			}
			std::cout << "===========================\n";


		}


	/*	for (auto elem = graph.mEdgeIdxMap.begin(); elem != graph.mEdgeIdxMap.end(); ++elem)
		{
			std::cout << elem->first << " ==== " << elem->second << "\n";

		}*/
		

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


	

	// function to reduce matrix to r.e.f. Returns a value to 
	// indicate whether matrix is singular or not 
	int forwardElim(std::vector < std::vector<bool>>& mat, std::vector<block>& y);

	// function to calculate the values of the unknowns 
	std::vector<block> backSub(std::vector < std::vector<bool>>& mat, std::vector<block>& y);

	// function to get matrix content 
	std::vector<block> gaussianElimination(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
	{
		int N = mat.size();
		/* reduction into r.e.f. */
		int singular_flag = forwardElim(mat,y);

		/* if matrix is singular */
		if (singular_flag != -1)
		{
			std::cout << ("Singular Matrix.\n");

			/* if the RHS of equation corresponding to
			zero row is 0, * system has infinitely
			many solutions, else inconsistent*/
			/*if (memcmp((u8*)&mat[singular_flag][N], (u8*)& ZeroBlock, sizeof(block))!=0 && )
				std::cout << ("Inconsistent System.");*/
		

			//return ;
		}

		/* get solution to system and print it using
		backward substitution */
		return backSub(mat,y);
	}

	// function for elementary operation of swapping two rows 
	void swap_row(std::vector < std::vector<bool>>& mat, std::vector<block>& y, int i, int j)
	{
		//printf("Swapped rows %d and %d\n", i, j); 
		int M= mat[0].size();

		for (int k = 0; k < M; k++)
		{
			bool temp = mat[i][k];
			mat[i][k] = mat[j][k];
			mat[j][k] = temp;

			
		}
		block temp1 = y[i];
		y[i] = y[j];
		y[j] = temp1;
	}

	// function to print matrix content at any stage 
	void print(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
	{
		std::cout << mat.size() << "\n";
		std::cout << mat[0].size() << "\n";

		for (int i = 0; i < mat.size(); i++, std::cout << ("\n"))
		{
			for (int j = 0; j < mat[i].size(); j++)
				std::cout << mat[i][j] << " ";
			
			std::cout << y[i] << " ";
		}
		std::cout<<("\n");
	}

	// function to reduce matrix to r.e.f. 
	int forwardElim(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
	{

		

		int N= mat.size();
		int M = mat[0].size();
		print(mat,y);
		for (int k = 0; k < N; k++)
		{
			// Initialize maximum value and index for pivot 
			int i_max = k;
			bool v_max = mat[i_max][k];

			/* find greater amplitude for pivot if any */
			for (int i = k + 1; i < N; i++)
				if (mat[i][k])
				{
					v_max = mat[i][k];
					i_max = i;
					break;
				}
			/* if a prinicipal diagonal element is zero,
			* it denotes that matrix is singular, and
			* will lead to a division-by-zero later. */
			if (!v_max) //all values at colum k =0 => skip this column 
			{
				std::cout << "diag " << k << " = 0 \n";
				continue;
			}
			/* Swap the greatest value row with current row */
			if (i_max != k)
				swap_row(mat, y,k, i_max);


			for (int i = k + 1; i < N; i++)
			{
				/* factor f to set current row kth element to 0,
				* and subsequently remaining kth column to 0 */
				//if (mat[i][k] == 0)
				if (!mat[i][k])
					continue;

			
				/* subtract fth multiple of corresponding kth
				row element*/
				for (int j = k; j < M; j++)
					mat[i][j] = mat[i][j] ^ mat[k][j];// -mat[k][j];

				y[i] = y[i] ^ y[k];

				/* filling lower triangular matrix with zeros*/
				//mat[i][k] = 0;
			}

			print(mat,y);	 //for matrix state 
		}
		print(mat,y);		 //for matrix state 
		return -1;
	}

	// function to calculate the values of the unknowns 
	std::vector<block> backSub(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
	{
		PRNG prng(ZeroBlock);
		

		int N = mat.size();
		int M = mat[0].size();

		std::vector<block>x(M); // An array to store solution 

		/* Start calculating from last equation up to the
		first */
		int previous_idx_non_zero = M;

		for (int i = N - 1; i >= 0; i--)
		{
			// find a first non-zero cell
			int idx_non_zero = -1;
			for (int j = i; j< M; j++)
				if (mat[i][j])
				{
					idx_non_zero = j;
					break;
				}

			if (idx_non_zero != -1) {
				
				block sum = ZeroBlock;
				for (int j = idx_non_zero+1; j < previous_idx_non_zero; j++)
				{
					x[j] = prng.get<block>();  // chose random value for all X[from idx_non_zero to previous_idx_non_zero]
					if (mat[i][j])
						sum = sum ^ x[j];
				}

				for (int j = previous_idx_non_zero; j < M; j++)
				{
					// these x[j] have been assigned before
					if (mat[i][j])
						sum = sum ^ x[j];
				}

				/* divide the RHS by the coefficient of the
				unknown being calculated */
				x[idx_non_zero] = (y[i]^sum );

				previous_idx_non_zero = idx_non_zero;
			}
		}

		std::cout << "\nSolution for the system:\n";
		for (int i = 0; i < M; i++)
			std::cout << x[i] << "\n";

		return x;
	}

	
	

	// Driver program 
	void Gaussian_Elimination_Test()
	{
		/* input matrix */
		
		std::vector < std::vector<bool>> mat(4);
		mat[0] = { 1,0,0,1,1};
		mat[1] = { 1,0,1,1,0 };
		mat[2] = { 1,0,1,0,0 };
		mat[3] = { 0,1,0,1,0 };

		std::vector<block> Y{ _mm_set_epi64x(0, 4), _mm_set_epi64x(0, 4), _mm_set_epi64x(0, 3), _mm_set_epi64x(0, 2) };
		
		auto x= gaussianElimination(mat,Y);

		for (int i = 0; i < mat.size(); i++)
		{
			block sum=ZeroBlock;
			for (u64 j = 0; j < mat[i].size(); j++)
			{
				if (mat[i][j])
					sum = sum ^ x[j];

			}
			if (memcmp((u8*)& sum, (u8*)& Y[i], sizeof(block)) != 0)
			{
				std::cout << sum << " vs " << Y[i] << "\n";
			}

		}
		//return 0;
	}
}