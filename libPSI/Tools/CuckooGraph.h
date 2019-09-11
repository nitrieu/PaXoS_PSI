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


	inline static void increasingSwap(u64& left, u64& right)
	{
		if (left > right)
			swap(left, right);
	}

	inline static string concateInts(u64 left, u64 right, u64 domain)
	{
		auto scale = left * (pow(10,floor(log10(domain)) + 1)); //scale
		return (ToString(scale) + ToString(right)); //h1||h2
	}

	
	inline static string concateIntsIncr(u64 left, u64 right, u64 domain)
	{
		increasingSwap(left, right);
		return concateInts(left, right, domain);
	}

	inline static string Edge2StringIncr(EdgeID edge, u64 domain)
	{
		auto a = edge.m_source;
		auto b = edge.m_target;
	
		//if (a > b)
			increasingSwap(a, b);

		return concateInts(a, b, domain);
	}


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
		boost::shared_ptr< std::vector<EdgeID>> mDfs_visitor; // TODO: remove this
		boost::shared_ptr< std::vector<EdgeID>> mDfs_component;
		boost::shared_ptr<std::vector<std::vector<EdgeID>>> mDfs_non_connected_components;
		boost::shared_ptr<std::vector<std::vector<EdgeID>>> mDfs_circles;
		//contain many connected components, each contains many circles, each circle contains manay edges + end at back_edge 
		boost::shared_ptr< std::vector<std::vector<std::vector<EdgeID>>>> mDfs_connected_components;
		boost::shared_ptr<std::set<EdgeID>> mDfs_back_edges;

	public:
		MyVisitor() : mDfs_visitor(new std::vector<EdgeID>())
			, mDfs_component(new std::vector<EdgeID>())
			, mDfs_non_connected_components(new std::vector<std::vector<EdgeID>>())
			, mDfs_circles(new std::vector<std::vector<EdgeID>>)
			, mDfs_connected_components(new std::vector<std::vector<std::vector<EdgeID>>>)
			, mDfs_back_edges(new std::set<EdgeID>()) {}

		
		AES mAesHasher;

		ccGraph mCuckooGraph;
		std::map<string, int> mEdgeIdxMap; //TODO: edgeId mapping // std::map<EdgeID, int> mEdgeIdxMap2;
		int mInputSize, mNumHashs, mNumBins;
		std::vector<u64> hashes1;
		std::vector<u64> hashes2;
		std::vector<string> strHashesIncr;
		std::set<string> mStrBadItems;
		std::set<int> mIdxBadItems; //for test


		void init(u64 inputSize, u64 numHashs, u64 numBins);
		void buidingGraph(span<block> items);

		/*void initialize_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
		std::cout << "initialize_vertex: " << s<< std::endl;
		}
		*/
		void start_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
			//std::cout << "start_vertex: " << s << std::endl;
			start_Vertex = s;
		//	isConnectedComponent = false;
		}

		void discover_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {

			//insertOrder.push_back(s);
			//verticesBFS.emplace(s, parent);
			//verticesDFS.emplace(s, parent);
		//	std::cout << "discover_vertex: " << s << std::endl;
		}
		void examine_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
			//parent = s;
			//std::cout << "examine_vertex: " << s << std::endl;
		}
		/*void examine_edge(const Graph::edge_descriptor &e, const Graph &g) const {
		std::cout << "Examine edge: " << e<< std::endl;
		}*/

		
		void tree_edge(const ccGraph::edge_descriptor& e, const ccGraph& g) const {
		
			auto strEdge=Edge2StringIncr(e, mNumBins);
			if (mStrBadItems.find(strEdge) != mStrBadItems.end())  //bad item
			{
				std::cout << "back_edge back_edge: " << e << std::endl;
				mDfs_back_edges->insert(e);
				mDfs_component->push_back(e); //last is back_edge
				mDfs_circles->push_back(*mDfs_component); // add this circle to dfs_circles
				mDfs_component->clear(); //for next circle
			}
			else {
				mDfs_visitor->push_back(e);
				mDfs_component->push_back(e);
				std::cout << "tree_edge: " << e << std::endl;
			}
		}

		

		void back_edge(const ccGraph::edge_descriptor& e, const ccGraph& g) const {

			// since the graph is undirected, if tree_edge is (0,1) then back_edge is (1,0)
			//we want to remove this back_edge by remembering dfs_tree_edges, and check the condition in void back_edge

		//	if (mDfs_visitor->find(e) == mDfs_visitor->end())
		  if(std::find(mDfs_visitor->begin(), mDfs_visitor->end(), e) == mDfs_visitor->end())
			{
				std::cout << "back_edge back_edge: " << e << std::endl;
				// find a real back_edge => this make a circle
				mDfs_back_edges->insert(e);
				mDfs_component->push_back(e); //last is back_edge
				mDfs_circles->push_back(*mDfs_component); // add this circle to dfs_circles
				mDfs_component->clear(); //for next circle
				//isCircle->clear(); //for next circle
			}



			//	std::cout << "dfs_tree_edge->m_target: " << dfs_tree_edge->m_target << std::endl;
			//*dfs_back_edge = e;

			std::cout << "back_edge: " << e << std::endl;
		}
		void forward_or_cross_edge(const ccGraph::edge_descriptor& e, const ccGraph& g) const {
			std::cout << "forward_or_cross_edge: " << e << std::endl;
			//if (mDfs_visitor->find(e) == mDfs_visitor->end() && mDfs_back_edges->find(e) == mDfs_back_edges->end())
			if (std::find(mDfs_visitor->begin(), mDfs_visitor->end(), e) == mDfs_visitor->end()
				&& mDfs_back_edges->find(e) == mDfs_back_edges->end())

			
			{
				std::cout << "back_edge back_edge: " << e << std::endl;
				// find a real back_edge => this make a circle
				mDfs_back_edges->insert(e);
				mDfs_component->push_back(e); //last is back_edge
				mDfs_circles->push_back(*mDfs_component); // add this circle to dfs_circles
				mDfs_component->clear(); //for next circle
				//isCircle->clear(); //for next circle
			}
		}

		std::vector<std::vector<EdgeID>>& GetDfsCircles() const { return *mDfs_circles; }
		std::set<EdgeID>& GetDfsBackEdge() const { return *mDfs_back_edges; }
		//std::vector<std::vector<EdgeID>>& GetDfsNOConnectedComponents() const { return *mDfs_non_connected_components; }
		std::vector<EdgeID>& GetDfsVisitor() const { return *mDfs_visitor; }
		
		void finish_vertex(const ccGraph::vertex_descriptor& s, const ccGraph& g) const {
			//std::cout << "finish_vertex: " << s << std::endl;
			
			//if (s== start_Vertex)
			//{
			//	std::cout << "finish_vertex finish_vertex: " << s << std::endl;
			//	std::cout << "mDfs_component->size(): " << mDfs_component->size() << std::endl;

			//	if (mDfs_component->size() > 0) //no circle
			//	{
			//		mDfs_non_connected_components->push_back(*mDfs_component); // 
			//		mDfs_component->clear(); //for next circle
			//	//	isConnectedComponent = false;
			//	}

			//	if (isConnectedComponent)
			//	{
			//		//if(mDfs_component->size() > 0))
			//		mDfs_connected_components->push_back(*mDfs_circles); // 
			//		mDfs_component->clear(); //for next circle
			//		mDfs_circles->clear(); //for next circle
			//		isConnectedComponent = false;
			//	}
			//}
		}

	};
}
