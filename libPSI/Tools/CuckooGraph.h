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


namespace osuCrypto
{
	typedef boost::adjacency_list<  // adjacency_list is a template depending on :
		boost::vecS,               //  The container used for egdes : here, std::list.
		boost::vecS,                //  The container used for vertices: here, std::vector.
		boost::undirectedS,           //  directed or undirected edges ?.
		int,                     //  The type that describes a Vertex.
		string                        //  The type that describes an Edge
	>ccGraph;


	typedef ccGraph::vertex_descriptor VertexID;
	typedef ccGraph::edge_descriptor   EdgeID;

	//typedef std::vector<pair<int, int>> DfsTreeEdges;
	//typedef pair<int, int> DfsTreeEdges;
	//typedef std::unordered_map<int, int> DfsVertices;


	


	/*
	std::vector<int> insertOrder;
	std::unordered_map<int, int> verticesBFS;
	std::unordered_map<int, int> verticesDFS;

	int parent;

	int mDfs_cnt_tree_edge;
	DfsTreeEdges mDfs_tree_edges;*/

	static VertexID start_Vertex;
	static bool isConnectedComponent;
	class MyVisitor : public boost::default_dfs_visitor
	{
	private:
		boost::shared_ptr< std::set<EdgeID>> mDfs_tree_all_edges; // TODO: remove this
		boost::shared_ptr< std::vector<EdgeID>> mDfs_component;
		boost::shared_ptr<std::vector<std::vector<EdgeID>>> mDfs_non_circles;
		boost::shared_ptr<std::vector<std::vector<EdgeID>>> mDfs_circles;
		

		//int a;
		/*boost::shared_ptr < pair<int, int>> dfs_tree_edge;
		pair<int, int> dfs_back_edge;*/
	//boost::shared_ptr < EdgeID> dfs_tree_edge;
	/*		boost::shared_ptr < EdgeID> dfs_back_edge;*/

	public:
	
		

		MyVisitor() : mDfs_tree_all_edges(new std::set<EdgeID>())
			, mDfs_component(new std::vector<EdgeID>())
			, mDfs_non_circles(new std::vector<std::vector<EdgeID>>())
			, mDfs_circles(new std::vector<std::vector<EdgeID>>) {}
		

		/*void initialize_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
		std::cout << "initialize_vertex: " << s<< std::endl;
		}
		*/
		void start_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
			std::cout << "start_vertex: " << s << std::endl;
			start_Vertex = s;
			isConnectedComponent = false;
		}

		void discover_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {

			//insertOrder.push_back(s);
			//verticesBFS.emplace(s, parent);
			//verticesDFS.emplace(s, parent);
			std::cout << "discover_vertex: " << s << std::endl;
		}
		void examine_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
			//parent = s;
			std::cout << "examine_vertex: " << s << std::endl;
		}
		/*void examine_edge(const Graph::edge_descriptor &e, const Graph &g) const {
		std::cout << "Examine edge: " << e<< std::endl;
		}*/

		void back_edge(const ccGraph::edge_descriptor& e, const ccGraph& g) const {


			// since the graph is undirected, if tree_edge is (0,1) then back_edge is (1,0)
			//we want to remove this back_edge by remembering dfs_tree_edges, and check the condition in void back_edge

			if (mDfs_tree_all_edges->find(e) == mDfs_tree_all_edges->end())
			{
				isConnectedComponent = true;//circle
				std::cout << "back_edge back_edge: " << e << std::endl;
				// find a real back_edge => this make a circle

				mDfs_component->push_back(e); //last is back_edge
				mDfs_circles->push_back(*mDfs_component); // add this circle to dfs_circles

				mDfs_component->clear(); //for next circle
				//isCircle->clear(); //for next circle
			}



			//	std::cout << "dfs_tree_edge->m_target: " << dfs_tree_edge->m_target << std::endl;
			//*dfs_back_edge = e;

			std::cout << "back_edge: " << e << std::endl;
		}
		void tree_edge(const ccGraph::edge_descriptor& e, const ccGraph& g) const {
		
			mDfs_tree_all_edges->insert(e);
			mDfs_component->push_back(e);

			std::cout << "tree_edge: " << e << std::endl;
			std::cout << "mDfs_component->size(): " << mDfs_component->size() << std::endl;
		}
		std::vector<std::vector<EdgeID>>& GetDfsCircles() const { return *mDfs_circles; }
		std::vector<std::vector<EdgeID>>& GetDfsNOCircles() const { return *mDfs_non_circles; }

		

		void forward_or_cross_edge(const ccGraph::edge_descriptor& e, const ccGraph& g) const {
			std::cout << "forward_or_cross_edge: " << e << std::endl;
		}
		void finish_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
			std::cout << "finish_vertex: " << s << std::endl;
			
			if (s== start_Vertex)
			{
				
				if (!isConnectedComponent && mDfs_component->size() > 0) //no circle
				{
					mDfs_non_circles->push_back(*mDfs_component); // 
					mDfs_component->clear(); //for next circle
				}
				std::cout << "finish_vertex finish_vertex: " << s << std::endl;
			}
		}

	};

	class CuckooGraph
	{
	public:



		void init(u64 inputSize, u64 numHashs, u64 numBins);
		void buidingGraph(span<block> items);
		AES mAesHasher;
		 std::vector<int> mIdx_inputs_circle_contains_2vertices;
		
		 ccGraph mCuckooGraph;
		 //std::unordered_map<EdgeID, int> mEdge_map;

	private:

		int mInputSize, mNumHashs, mNumBins;
		std::vector<u64> mHashes;
		


	};
}
