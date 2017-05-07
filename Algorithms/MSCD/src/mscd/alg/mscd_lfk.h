/*
 *  mscd_lfk.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 24/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_LFK_H_
#define MSCD_LFK_H_

// Includes
#include "mscd_algorithm.h"
#include <set>
#include <algorithm>
#include <map>
#include <queue>
#include <cmath>
#include "graph.h"

//#include <chrono>
//#include <iostream>

namespace mscd {
namespace alg {

/** Multi-scale community detection using Lancichinetti, Fortunato and Kertész's method.
 *	The implementation assumes the network is undirected for optimisation purposes.
 */
class MSCD_LFK: public MSCDAlgorithm {
	
public:
	
	//typedef std::chrono::high_resolution_clock clock;
	//clock::time_point i0, g0, c0, m0;
	//std::chrono::milliseconds dur_i, dur_g, dur_c, dur_m;
	
	/// Default constructor
	MSCD_LFK() : merge_th(0.5), max_nb_failures(0), max_check_loops(0) {}
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "LFK"; }
	
	/// Run the algorithm
	virtual bool Run(const ds::Graph &,
					 const std::vector<double> &,
					 std::vector<ds::Communities> &,
					 std::vector<double> &,
					 const ds::Communities &);
	
	/** Set extra parameters from a list of values
	 *	1. merging threshold in [0,1]
	 */
	virtual bool SetExtraParams(const std::vector<double> & params) {
		if (!params.empty()) {
			if (params.size() > 3) {
				fprintf(stderr, "Too many arguments\n");
				return false;
			}
			if ((params[0] < 0.) && (params[0] > 1.)) {
				fprintf(stderr, "Parameter 1: Merging threshold must be within [0,1]\n Incorrect value %f\n", params[0]);
				return false;
			}
            if ((params.size() >= 2) && (params[1] < 0.)) {
				fprintf(stderr, "Parameter 2: Max neighbour failures must be positive\n Incorrect value %d\n", static_cast<int>(params[1]));
				return false;
			}
			if ((params.size() >= 3) && (params[2] < 0.)) {
				fprintf(stderr, "Parameter 3: Max check loops must be positive\n Incorrect value %d\n", static_cast<int>(params[2]));
				return false;
			}
			this->SetMergeTh(params[0]);
			if (params.size() >= 2) this->SetMaxNbFailures(static_cast<long>(params[1]));
            if (params.size() >= 3) this->SetMaxCheckLoops(static_cast<long>(params[2]));
			return true;
		}
		return false;
	}
	
	/// Set the merge threshold
	inline void SetMergeTh(const double value) { this->merge_th = value; }
	
	// Return the merge threshold
	inline double GetMergeTh() const { return this->merge_th; }
    
	/// Set the maximum number of neighbours not added during growth that do not stop the growth process
	inline void SetMaxNbFailures(const long value) { if (value >= 0) this->max_nb_failures = value; }
	
	// Return the maximum number of neighbours not added during growth that do not stop the growth process
	inline long GetMaxNbFailures() const { return this->max_nb_failures; }
	
    /// Set the maximum checking loops in the growth function
	inline void SetMaxCheckLoops(const long value) { if (value >= 0) this->max_check_loops = value; }
	
	// Return the maximum checking loops in the growth function
	inline long GetMaxCheckLoops() const { return this->max_check_loops; }
	
private:

	/// Merge threshold
	double merge_th;
	
	/// Maximum number of neighbours not added during growth that do not stop the growth process
	long max_nb_failures;
	
	/// Maximum number of loops when checking nodes should still belong to the community in the grow function
	long max_check_loops;
	
	/// Bool vectors used to speed up community membership.
	/// (They are assigned to false for all bits and should always return to this state after use.)
	std::vector<bool> bitvector, bitvector2;
	
	/// Grow the given community
	bool grow(const ds::Graph::adjlist &, double, std::vector<long> &, double &, double &, std::set<long> &);
	
    /// Return true if the first community encompasses the second
	inline bool encompass(const std::vector<long> & c1, const std::vector<long> & c2) const {
		if ((c1.back() < c2.front()) || (c2.back() < c1.front())) return false;
        else if (c2.size() == 1) return std::binary_search(c1.begin(), c1.end(), c2[0]); // 1st growths may face this case
		long i=0, j=0;
		while ((i<c1.size()) && (j<c2.size())) {
			while ((i<c1.size()) && (c1[i] < c2[j])) ++i;
			if (i == c1.size()) break;
			if (j<c2.size()) {
                if (c2[j] < c1[i]) return false;
                else if (c1[i] == c2[j]) { ++i; ++j; }
            }
		}
		return (j == static_cast<long>(c2.size()));
	}

	/// Compute the number of shared nodes between two communities (sorted lists) if beyond the minimum number required (otherwise return 0)
	inline long nb_shared_nodes(const std::vector<long> & c1, const std::vector<long> & c2) const {
		if ((c1.back() < c2.front()) || (c2.back() < c1.front())) return 0;
		long i=0, j=0, n=0;//, nmin=static_cast<long>(std::ceil(this->merge_th*(c1.size()<c2.size()?c1.size():c2.size())));
		while ((i<c1.size()) && (j<c2.size())) {
			while ((i<c1.size()) && (c1[i] < c2[j])) ++i;
			if (i == c1.size()) break;
			while ((j<c2.size()) && (c2[j] < c1[i])) ++j;
			if ((j<c2.size()) && (c1[i] == c2[j])) { ++i; ++j; ++n; }//--nmin; }
            //if (static_cast<long>(std::min(c1.size()-i, c2.size()-j)) < nmin) return 0; // exit condition if threshold cannot be reached
		}
		return n;
	}
	
	/// Return true if the given community component is not broken by the removal of the given node
	inline bool isStillConnected(const ds::Graph::adjlist & al, const std::vector<long> & com, const std::vector<bool> & is_com_node, const long n) {
		if (com.size() <= 2) return true; // If one or zero elements left: it is connected
		std::queue<long> queue;
		std::vector<bool> & is_visited = this->bitvector2;
		std::vector<long> visited(com.size());
		long i, next, nb, nb_visited = 0;
		is_visited[n].flip(); // Take n out
		visited[nb_visited++] = n;
		// Enqueue first node
		if (com[0] != n) queue.push(com[0]);
		else queue.push(com[1]);
		is_visited[queue.front()].flip();
		visited[nb_visited++] = queue.front();
		// While there are nodes to visit
		while (!queue.empty()) {
			next = queue.front();
			queue.pop();
			// For each neighbour enqueue them if not visited
			for (i=0; i<al[next].size(); ++i) {
				nb = al[next][i].first;
				if ((is_com_node[nb]) && (!is_visited[nb])) {
					is_visited[nb].flip();
					visited[nb_visited++] = nb;
					queue.push(nb);
				}
			}
		}
		for (i=0; i<nb_visited; ++i) is_visited[visited[i]].flip(); // Clean bit vector
		return (nb_visited==com.size());
	}
	
#ifdef __DEBUG__
	void DbCheckKs(const ds::Graph::adjlist &, const std::vector<long> &, double &, double &) const;
#endif

};

} // namespace alg
} // namespace mscd

#endif

/*
 inline bool isStillConnected(const ds::Graph::adjlist & al, const std::vector<long> & com, const long n) const {
 // If one or zero elements left: connected
 if (com.size() <= 2) return true;
 std::map<long,bool> visited;
 std::queue<long> queue;
 // Take n out
 for (long i=0; i<com.size(); ++i) visited[com[i]] = (com[i]==n)?true:false;		
 // Enqueue first node
 if (com[0] != n) queue.push(com[0]);
 else queue.push(com[1]);
 visited[queue.front()] = true;
 // While there are nodes to visit
 long next, nb, i, n_visited = 1;
 while (!queue.empty()) {
 next = queue.front();
 queue.pop();
 // For each neighbour enqueue them if not visited
 for (i=0; i<al[next].size(); ++i) {
 nb = al[next][i].first;
 if (std::binary_search(com.begin(), com.end(), nb) && (!visited[nb])) {
 visited[nb] = true;
 queue.push(nb);
 ++n_visited;
 }
 }
 }
 //printf("%d %d\n", n_visited, com.size());
 return (n_visited==com.size()-1);
 }*/

/*
 inline bool isStillConnected(const ds::Graph::adjlist & al, const fast_set<long> & commap, const long n) const {
 //printf("IN\n");
 if (commap.size() <= 2) {printf("AII\n"); return true; } 
 // If one or zero elements left: connected
 fast_set<long> visited;
 std::queue<long> queue;
 // Take n out
 visited.insert(n);
 // Enqueue first node
 fast_set<long>::const_iterator it = commap.begin();
 if (*it != n) queue.push(*it);
 else queue.push(*(++it));
 visited.insert(queue.front());
 // While there are nodes to visit
 long i, next, nb, n_visited = 1;
 while (!queue.empty()) {
 next = queue.front();
 queue.pop();
 // For each neighbour enqueue them if not visited
 for (i=0; i<al[next].size(); ++i) {
 nb = al[next][i].first;
 if ((commap.find(nb) != commap.end()) && (visited.find(nb) == visited.end())) {
 //if (std::binary_search(com.begin(), com.end(), nb) && (!visited[nb])) {
 visited.insert(nb);
 queue.push(nb);
 ++n_visited;
 }
 }
 }
 //printf("%d %d\n", n_visited, com.size());
 //printf("OUt\n");
 return (n_visited==commap.size()-1);
 }*/
