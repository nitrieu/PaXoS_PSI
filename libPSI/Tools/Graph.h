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

namespace osuCrypto
{
	typedef boost::adjacency_list<  // adjacency_list is a template depending on :
		boost::vecS,               //  The container used for egdes : here, std::list.
		boost::vecS,                //  The container used for vertices: here, std::vector.
		boost::undirectedS,           //  directed or undirected edges ?.
		int,                     //  The type that describes a Vertex.
		string                        //  The type that describes an Edge
	>Graph;


	typedef Graph::vertex_descriptor VertexID;
	typedef Graph::edge_descriptor   EdgeID;

	std::vector<int> insertOrder;
	std::unordered_map<int, int> verticesBFS;
	std::unordered_map<int, int> verticesDFS;

	int parent;

	int dfs_cnt_tree_edge;
	std::vector<pair<int, int>> dfs_tree_edges;

	class MyVisitor : public boost::default_dfs_visitor
	{
	private:
	public:
		/*void initialize_vertex(const graph_t::vertex_descriptor &s, const graph_t &g) const {
		std::cout << "Initialize: " << s<< std::endl;
		}*/
		void discover_vertex(const Graph::vertex_descriptor& s, const Graph& g) const {

			insertOrder.push_back(s);
			verticesBFS.emplace(s, parent);
			verticesDFS.emplace(s, parent);
			//std::cout << "Discover: " << s << std::endl;
		}
		void examine_vertex(const Graph::vertex_descriptor& s, const Graph& g) const {
			parent = s;
			//	std::cout << "Examine vertex: " << s << std::endl;
		}
		/*void examine_edge(const Graph::edge_descriptor &e, const Graph &g) const {
		std::cout << "Examine edge: " << e<< std::endl;
		}*/
		void tree_edge(const Graph::edge_descriptor& e, const Graph& g) const {
			dfs_cnt_tree_edge++;
			dfs_tree_edges.push_back(make_pair(e.m_source, e.m_target));

			//std::cout << "Tree edge: " <<e << std::endl;
		}
		void back_edge(const Graph::edge_descriptor& e, const Graph& g) const {
			//std::cout << "back edge: " << e << std::endl;

		}

		void non_tree_edge(const Graph::edge_descriptor& e, const Graph& g) const {
			std::cout << "Non-Tree edge: " << e << std::endl;
		}
		void gray_target(const Graph::edge_descriptor& e, const Graph& g) const {
			//std::cout << "Gray target: " << e << std::endl;
		}
		void black_target(const Graph::edge_descriptor& e, const Graph& g) const {
			std::cout << "Black target: " << e << std::endl;
		}
		void forward_or_cross_edge(const Graph::edge_descriptor& e, const Graph& g) const {
			//std::cout << "forward_or_cross_edge: " << e << std::endl;
		}
		void finish_vertex(const Graph::vertex_descriptor& s, const Graph& g) const {
			//std::cout << "Finish vertex: " << s << std::endl;
		}

	};

	class CuckooGraph
	{
	public:

		
		
		void init(u64 inputSize, u64 numHashs);
		void insertItems(span<block> items);

	private:

		int mInputSize, mNumHashs, mNumBins;
		std::vector<u64> mHashes;

	};


}
