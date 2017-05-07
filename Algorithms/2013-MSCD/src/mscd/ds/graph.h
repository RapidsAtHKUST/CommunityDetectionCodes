/*
 *  graph.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 28/11/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_GRAPH_H_
#define MSCD_GRAPH_H_

// Includes
#include "config.h"
#include <vector>

namespace mscd {
namespace ds {	

/** Graph class using an adjacency list representation.
 *	The graph can be weighted but not multi-edged (a multi-edge graph can ususally be represented as a weighted graph).
 *	To nodes are accessed in the public methods using a 1-based indexing.
 */
class Graph {
	
public:
	
	/// Short typename for the graph adjacency list
	typedef std::vector<std::vector<std::pair<long,preal> > > adjlist;
	
	/// Default constructor
	Graph(): nb_edges(0), total_weight(0.), directed(false) {}
	
	/// Construct a directed or undirected graph
	Graph(const bool dir = false): nb_edges(0), total_weight(0.), directed(dir) {}
	
	/// Construct a graph from a number of nodes and precising whether it is directed or not
	Graph(const long nb_nodes, const bool dir = false): adj_list(nb_nodes), nb_edges(0), total_weight(0.), directed(dir) {}
	
	/// Resize a graph to a larger size
	inline bool Resize(const long nb_nodes) {
		if (nb_nodes <= this->adj_list.size()) return false;
		this->adj_list.resize(nb_nodes);
		return true;
	}
	
	/// Return true if the graph is directed
	inline bool IsDirected() const { return this->directed; }
	
	/// Return the graph adjacency adjacency list
	inline const std::vector<std::vector<std::pair<long,preal> > > & GetAdjList() const { return this->adj_list; }
	
	/// Return the number of nodes
	inline long GetNbNodes() const { return this->adj_list.size(); }
	
	/// Return the number of edges of the graph
	inline long GetNbEdges() const { return this->nb_edges; }
	
	/// Return the total weight of the graph (sum of all edges)
	inline double GetTotalWeight() const { return this->total_weight; }
	
	/// Return the number of edges of the given node (an undirected edge counts only once)
	inline long GetNodeNbEdges(const long n) const {
		if ((n < 1) || (n > this->adj_list.size())) return -1;
		return this->adj_list[n-1].size();
	}
	
	/// Return the target of the given edge for the given node
	inline long GetTarget(const long n, const long edge_n) const {
		if ((n < 1) || (n > this->adj_list.size())) return -1;
		if ((edge_n < 1) || (edge_n > this->adj_list[n-1].size())) return -1;
		return this->adj_list[n-1][edge_n-1].first+1;
	}
	
	/// Add the given edge (when not directed, edges are added in both directions)
	inline bool AddEdge(const long src, const long trg, const preal w = 1.f) {
		if ((src < 1) || (trg < 1)) return false;
		long m = (src>trg)?src:trg;
		if (m > this->adj_list.size()) this->adj_list.resize(m);
		if (!this->insert_edge(src-1, trg-1, w)) return false;
		if ((!directed) && (src != trg) && (!this->insert_edge(trg-1, src-1, w))) return false;
		++this->nb_edges;
		if (directed) this->total_weight += static_cast<double>(w);
		else this->total_weight += 2.*(static_cast<double>(w));
		return true;
	}

private:
	
	/// Adjacency list: for each node, a list of pairs (target, weight)
	std::vector<std::vector<std::pair<long,preal> > > adj_list;
	
	/// Number of edges
	long nb_edges;
	
	/// Total weight (sum of edges weight, twice edges weights if undirected)
	double total_weight;
				 
	/// Directed
	bool directed;
	
	/// Insert a valid edge in a sorted array (0-indexed)
	inline bool insert_edge(const long s, const long t, const preal w) {
		std::vector<std::pair<long,preal> >::iterator itor = this->adj_list[s].begin();
		while ((itor != this->adj_list[s].end()) && (itor->first < t)) ++itor;
		if ((itor != this->adj_list[s].end()) && (itor->first == t)) return false;
		this->adj_list[s].insert(itor, std::pair<long,float>(t,w));
		return true;
	}
	
};

} // namespace ds
} // namespace mscd

#endif