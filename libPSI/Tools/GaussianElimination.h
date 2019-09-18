#pragma once
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Log.h"
#include "cryptoTools/Common/BitVector.h"

#include "cryptoTools/Common/MatrixView.h"
//#include <mutex>
#include <atomic>

using namespace boost;
using namespace std;
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <set> 

#include "Tools/CuckooGraph.h"

namespace osuCrypto {
	//TODO: cleaning 
	inline static int forwardElim(std::vector < std::vector<bool>>& mat, std::vector<block>& y);

	// function to calculate the values of the unknowns 
	inline static std::vector<block> backSub(std::vector < std::vector<bool>>& mat, std::vector<block>& y);

	// function to get matrix content 
	inline static std::vector<block> gaussianElimination(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
	{
		int N = mat.size();
		/* reduction into r.e.f. */
		int singular_flag = forwardElim(mat, y);

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
		return backSub(mat, y);
	}

	// function for elementary operation of swapping two rows 
	inline static void swap_row(std::vector < std::vector<bool>>& mat, std::vector<block>& y, int i, int j)
	{
		//printf("Swapped rows %d and %d\n", i, j); 
		int M = mat[0].size();

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
	inline static void printMatrix(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
	{
	/*	std::cout << mat.size() << "\n";
		std::cout << mat[0].size() << "\n";

		for (int i = 0; i < mat.size(); i++, std::cout << ("\n"))
		{
			for (int j = 0; j < mat[i].size(); j++)
				std::cout << mat[i][j] << " ";

			std::cout << y[i] << " ";
		}
		std::cout << ("\n");*/
	}

	// function to reduce matrix to r.e.f. 
	inline static int forwardElim(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
	{



		int N = mat.size();
		int M = mat[0].size();
		printMatrix(mat, y);
		int colIdx = 0;
		int rowId = 0;
		while (rowId < N)
		{
			// Initialize maximum value and index for pivot 
			int i_max = rowId;
			bool v_max = mat[i_max][colIdx];

			/* find greater amplitude for pivot if any */
			for (int i = rowId; i < N; i++)
				if (mat[i][colIdx])
				{
					v_max = mat[i][colIdx];
					i_max = i;
					break;
				}
			/* if a prinicipal diagonal element is zero,
			* it denotes that matrix is singular, and
			* will lead to a division-by-zero later. */
			if (!v_max) //all values at colum k =0 => skip this column 
			{
				std::cout << "diag " << colIdx << " = 0 \n";
				colIdx++;
				continue;
			}
			/* Swap the greatest value row with current row */
			if (i_max != rowId)
				swap_row(mat, y, rowId, i_max);


			for (int i = rowId + 1; i < N; i++)
			{
				/* factor f to set current row kth element to 0,
				* and subsequently remaining kth column to 0 */
				//if (mat[i][k] == 0)
				if (!mat[i][colIdx])
					continue;


				/* subtract fth multiple of corresponding kth
				row element*/
				for (int j = colIdx; j < M; j++)
					mat[i][j] = mat[i][j] ^ mat[rowId][j];// -mat[k][j];

				y[i] = y[i] ^ y[rowId];

				/* filling lower triangular matrix with zeros*/
				//mat[i][k] = 0;
			}

			printMatrix(mat, y);	 //for matrix state 
			rowId++;
		}
		printMatrix(mat, y);		 //for matrix state 
		return -1;
	}

	// function to calculate the values of the unknowns 
	inline static std::vector<block> backSub(std::vector < std::vector<bool>>& mat, std::vector<block>& y)
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
			for (int j = i; j < M; j++)
				if (mat[i][j])
				{
					idx_non_zero = j;
					break;
				}

			if (idx_non_zero != -1) {

				block sum = ZeroBlock;
				for (int j = idx_non_zero + 1; j < previous_idx_non_zero; j++)
				{
					x[j] = prng.get<block>();  // chose random value for all X[from idx_non_zero to previous_idx_non_zero]

					//std::cout << "x[" << j << "]= " << x[j] << "\n";
					if (mat[i][j])
						sum = sum ^ x[j];

					//std::cout << "sum= " << sum << "\n";

				}

				for (int j = previous_idx_non_zero; j < M; j++)
				{
					// these x[j] have been assigned before
					if (mat[i][j])
						sum = sum ^ x[j];

				//	std::cout << "sum= " << sum << "\n";
				}

				/* divide the RHS by the coefficient of the
				unknown being calculated */
				x[idx_non_zero] = (y[i] ^ sum);
				std::cout << "xx[" << idx_non_zero << "]= " << x[idx_non_zero] << "\n";

				previous_idx_non_zero = idx_non_zero;
			}
		}

		//std::cout << "\nSolution for the system:\n";
		//for (int i = 0; i < M; i++)
		//	std::cout << x[i] << "\n";

		return x;
	}




	inline static void Cuckoo_encode(span<block> inputs, MyVisitor& graph, int numBin, int sigma)
	{
		u64 setSize = inputs.size();
		PRNG prng(ZeroBlock);

		//MyVisitor graph;
		graph.init(setSize, 2, numBin, sigma);
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
		for (int i = 0; i < inputs.size(); i++)
		{
			graph.functionR[i] = mAesRfunction.ecbEncBlock(inputs[i]);
		}

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
				equation = equation ^ graph.functionR[idx_item_in_circle];
				assocated_value = assocated_value ^ inputs[idx_item_in_circle];

				BitVector coeff((u8*)& equation, graph.mSigma);
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

		if (GaussMatrix.size() > 0)
			graph.R = gaussianElimination(GaussMatrix, assocated_values);

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
					sum = sum ^ graph.R[j];
			}

			if (neq(sum, copy_assocated_values[i]))
			{
				std::cout << sum << " sum vs " << copy_assocated_values[i] << "\n";
				std::cout << "failed " << i << std::endl;
				//throw UnitTestFail();
			}

		}

		//================Fill D

		bool isRoot;
		for (auto it = dfs_visitor.begin(); it != dfs_visitor.end(); ++it)
		{
			auto edge = *it;
			auto keyEgdeMapping = Edge2StringIncr(edge, graph.mNumBins);
			auto idxItem = graph.mEdgeIdxMap[keyEgdeMapping];

			bool isRoot = eq(graph.L[edge.m_source], AllOneBlock);
			if (isRoot) //root
			{
				graph.L[edge.m_source] = prng.get<block>();
				std::cout << idxItem << " , ";

			}



			//compute h2
			if (eq(graph.L[edge.m_target], AllOneBlock))
			{
				graph.L[edge.m_target] = graph.L[edge.m_source] ^ inputs[idxItem];
				block valueR = graph.functionR[idxItem];
				BitVector coeff((u8*)& valueR, graph.mSigma);
				for (int b = 0; b < coeff.size(); b++)
				{
					if (coeff[b])
						graph.L[edge.m_target] = graph.L[edge.m_target] ^ graph.R[b];
				}
			}
			else {
				std::cout << " \n " << idxItem << " idx ";

				cout << edge.m_target << " L: " << graph.L[edge.m_target] << " already fixed \n";

			}
		}

#if 0
		//==========decode
		for (int i = 0; i < inputs.size(); ++i)
		{
			auto h1 = graph.hashes1[i];
			auto h2 = graph.hashes2[i];
			auto x = graph.L[h1] ^ graph.L[h2];

			block valueR = graph.functionR[i];
			BitVector coeff((u8*)& valueR, graph.mSigma);
			//std::cout << coeff << "\n";
			for (int b = 0; b < coeff.size(); b++)
			{
				if (coeff[b])
				{
					//std::cout << coeff[b] << " coeff[b]\n";
					x = x ^ graph.R[b];
				}
			}
			if (neq(x, inputs[i]))
			{
				std::cout << i << ":" << x << " decode vs " << inputs[i] << "\n";
			}

		}

#endif
	}

	inline static void Cuckoo_decode(std::vector<block>& outputs, MyVisitor& graph)
	{
		//==========decode
		outputs.resize(graph.mInputSize);
		for (int i = 0; i < graph.mInputSize; ++i)
		{
			auto h1 = graph.hashes1[i];
			auto h2 = graph.hashes2[i];
			outputs[i] = graph.L[h1] ^ graph.L[h2];

			block valueR = graph.functionR[i];
			BitVector coeff((u8*)& valueR, graph.mSigma);
			//std::cout << coeff << "\n";
			for (int b = 0; b < coeff.size(); b++)
			{
				if (coeff[b])
				{
					//std::cout << coeff[b] << " coeff[b]\n";
					outputs[i] = outputs[i] ^ graph.R[b];
				}
			}
			

		}


	}


}