//
//  mscd_lfk2.h
//  MSCD
//
//  Created by Erwan Le Martelot on 18/04/2012.
//  Copyright (c) 2012 Erwan Le Martelot. All rights reserved.
//

#ifndef MSCD_LFK2_H_
#define MSCD_LFK2_H_

// Includes
#include "mscd_algorithm.h"

// This implementation requires multi-threading
#ifdef USE_MULTITHREADING

#include <set>
#include <algorithm>
#include <map>
#include <queue>
#include <cmath>
#include "graph.h"
#include "atomic_set.h"

//#include <chrono>
//#include <iostream>

namespace mscd {
namespace alg {
		
/** Multi-scale community detection using Lancichinetti, Fortunato and Kertész's method.
 *	The implementation is parallelised using multi-threading and assumes the network is undirected for optimisation purposes.
 */
class MSCD_LFK2: public MSCDAlgorithm {

public:
	
	//typedef std::chrono::high_resolution_clock clock;
	//clock::time_point i0, r0, g0, c0, m0, e0;
	//std::chrono::milliseconds dur_i, dur_r, dur_g, dur_c, dur_m, dur_e;
	
	/// Default constructor
	MSCD_LFK2() : merge_th(0.5), max_nb_threads(0), max_nb_seeds(0), seed_ngh_level(1) {}
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "LFK2"; }
	
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
			if (params.size() > 4) {
				fprintf(stderr, "Too many arguments\n");
				return false;
			}
			if ((params[0] < 0.) && (params[0] > 1.)) {
				fprintf(stderr, "Parameter 1: Merging threshold must be within [0,1]\n Incorrect value %f\n", params[0]);
				return false;
			}
			if ((params.size() >= 2) && (params[1] < 0.)) {
				fprintf(stderr, "Parameter 2: Maximum number of threads must be positive (or 0 by default for hardware-based value)\n Incorrect value %f\n", params[1]);
				return false;
			}
			if ((params.size() >= 3) && (params[2] < 0.)) {
				fprintf(stderr, "Parameter 3: Maximum number of seeds must be positive (or 0 for infinite)\n Incorrect value %f\n", params[2]);
				return false;
			}
			if ((params.size() >= 4) && (params[3] < 0) && (params[3] > 2)) {
				fprintf(stderr, "Parameter 4: Recursive level of seed neighbours not to use as seeds must be 0, 1 or 2 (default: 1)\n Incorrect value %f\n", params[3]);
				return false;
			}
			this->SetMergeTh(params[0]);
			if (params.size() >= 2) this->SetMaxNbThreads(static_cast<long>(params[1]));
            if (params.size() >= 3) this->SetMaxNbSeeds(static_cast<long>(params[2]));
			if (params.size() >= 4) this->SetSeedNghLevel(static_cast<int>(params[3]));
			return true;
		}
		return false;
	}
	
	/// Set the merge threshold
	inline void SetMergeTh(const double value) { this->merge_th = value; }
	
	// Return the merge threshold
	inline double GetMergeTh() const { return this->merge_th; }
	
	/// Set the maximum number of additional threads that can be laucnhed in parallel
	inline void SetMaxNbThreads(const long value) { if (value >= 0) this->max_nb_threads = value; }
	
	/// Return the maximum number of additional threads that can be laucnhed in parallel
	inline long GetMaxNbThreads() const { return this->max_nb_threads; }

	/// Set the maximum number of seeds for growing the initial communities
	inline void SetMaxNbSeeds(const long value) { if (value >= 0) this->max_nb_seeds = value; }
	
	/// Return the maximum number of seeds for growing the initial communities
	inline long GetMaxNbSeeds() const { return this->max_nb_seeds; }
	
	/// Set the recursive level of seed neighbours not to use as seeds for growing the initial communities
	inline void SetSeedNghLevel(const int value) { if ((value >= 0) && (value <= 2)) this->seed_ngh_level = value; }
	
	/// Return the recursive level of seed neighbours not to use as seeds for growing the initial communities
	inline int GetSeedNghLevel() const { return this->seed_ngh_level; }


private:
	
	/// Merge threshold
	double merge_th;
	
	/// Maximum number of additional threads that can be laucnhed in parallel
	long max_nb_threads;
	
	/// Maximum number of seeds for growing the initial communities
	long max_nb_seeds;
	
	/// Recursive level of seed neighbours not to use as seeds
	int seed_ngh_level;
	
	/// Grow the given community
	bool grow(const ds::Graph::adjlist &, const double, const long, std::vector< toolkit::atomic_set<long> > &, std::vector<long> &, double &, double &, std::set<long> &, std::vector<bool> &, std::vector<bool> &);
	
	/// Check the given community
	void check(const ds::Graph::adjlist &, const long, const std::vector<long> &, std::vector< toolkit::atomic_set<long> > &, std::set< std::pair<long,long> > &);
	
	/// Merge the two given communities
	void merge(const ds::Graph::adjlist &, const long, const long, std::vector<long> &, std::set<long> &, double &, double &, std::vector<long> &, std::set<long> &, double &, double &, std::vector<bool> &, std::vector<bool> &);
	
	/// Compute the number of shared nodes between two communities (sorted lists) if beyond the minimum number required (otherwise return 0)
	inline long nb_shared_nodes(const std::vector<long> & c1, const std::vector<long> & c2) const {
		if ((c1.back() < c2.front()) || (c2.back() < c1.front())) return 0;
		long i=0, j=0, n=0;
		while ((i<c1.size()) && (j<c2.size())) {
			while ((i<c1.size()) && (c1[i] < c2[j])) ++i;
			if (i == c1.size()) break;
			while ((j<c2.size()) && (c2[j] < c1[i])) ++j;
			if ((j<c2.size()) && (c1[i] == c2[j])) { ++i; ++j; ++n; }
		}
		return n;
	}

	/// Return true if the given community component is not broken by the removal of the given node
	inline bool isStillConnected(const ds::Graph::adjlist & al, const std::vector<long> & com, const std::vector<bool> & is_com_node, const long n, std::vector<bool> & allocated_bitvector) {
		if (com.size() <= 2) return true; // If one or zero elements left: it is connected
		std::queue<long> queue;
		std::vector<bool> & is_visited = allocated_bitvector;		
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
		return true;
	}
	
	// Data struture passed to threads to call the grow function
	struct ThreadGrowData {
		ThreadGrowData(const ds::Graph::adjlist & al,
					   const double a,
					   const std::vector<long> & coms_ids,
					   const long begin,
					   const long end,
					   std::vector< toolkit::atomic_set<long> > & ncoms,
					   std::vector<std::vector<long> > & coms,
					   std::vector<double> & kin,
					   std::vector<double> & ktot,
					   std::vector< std::set<long> > & nbs,
					   std::vector<bool> & bv1,
					   std::vector<bool> & bv2
					   ) : al(al), a(a), coms_ids(coms_ids), begin(begin), end(end), ncoms(ncoms), coms(coms), kin(kin), ktot(ktot), nbs(nbs), bv1(bv1), bv2(bv2), ret_value(end-begin) {}
		const ds::Graph::adjlist & al;
		const double a;
		const std::vector<long> & coms_ids;
		const long begin, end;
		std::vector< toolkit::atomic_set<long> > & ncoms;
		std::vector<std::vector<long> > & coms;
		std::vector<double> & kin;
		std::vector<double> & ktot;
		std::vector< std::set<long> > & nbs;
		std::vector<bool> & bv1, & bv2;
		std::vector<bool> ret_value;
	};
	// Allows threads to call the grow function on instances
	friend void thread_grow_func(MSCD_LFK2 & inst, ThreadGrowData & data);
	
	// Data struture passed to threads to call the check function
	struct ThreadCheckData {
		ThreadCheckData(const ds::Graph::adjlist & al,
						const std::set<long> & check_coms,
						const long begin,
						const long nb,
						const std::vector<std::vector<long> > & coms,
						std::vector< toolkit::atomic_set<long> > & ncoms
						) : al(al), check_coms(check_coms), begin(begin), nb(nb), ncoms(ncoms), coms(coms) {}
		const ds::Graph::adjlist & al;
		const std::set<long> & check_coms;
		const long begin, nb;
		const std::vector<std::vector<long> > & coms;
		std::vector< toolkit::atomic_set<long> > & ncoms;
		std::set< std::pair<long,long> > to_merge;
	};
	// Allows threads to call the check function on instances
	friend void thread_check_func(MSCD_LFK2 & inst, ThreadCheckData & data);
	
	// Data struture passed to threads to call the merge function
	struct ThreadMergeData {
		ThreadMergeData(const ds::Graph::adjlist & al,
						const std::set< std::pair<long,long> > & thr_ops,
						std::vector<std::vector<long> > & coms,
						std::vector<double> & kin,
						std::vector<double> & ktot,
						std::vector< std::set<long> > & nbs,
						std::vector<bool> & bv1,
						std::vector<bool> & bv2
						) : al(al), thr_ops(thr_ops), coms(coms), kin(kin), ktot(ktot), nbs(nbs), bv1(bv1), bv2(bv2) {}
		const ds::Graph::adjlist & al;
		const std::set< std::pair<long,long> > & thr_ops;
		std::vector<std::vector<long> > & coms;
		std::vector<double> & kin, & ktot;
		std::vector< std::set<long> > & nbs;
		std::vector<bool> & bv1, & bv2;
	};
	// Allows threads to call the merge function on instances
	friend void thread_merge_func(MSCD_LFK2 & inst, ThreadMergeData & data);
	
#ifdef __DEBUG__
	void DbCheckKs(const ds::Graph::adjlist &, const std::vector<long> &, double &, double &) const;
#endif
	
};
		
} // namespace alg
} // namespace mscd

#endif

#endif
