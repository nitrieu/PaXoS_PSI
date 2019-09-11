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
					x[j] = ZeroBlock;// prng.get<block>();  // chose random value for all X[from idx_non_zero to previous_idx_non_zero]

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


	// function to reduce matrix to r.e.f. Returns a value to 
	// indicate whether matrix is singular or not 

}