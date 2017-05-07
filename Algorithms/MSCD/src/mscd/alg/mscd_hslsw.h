/*
 *  mscd_hslsw.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 25/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_HSLSW_H_
#define MSCD_HSLSW_H_

// Includes
#include "mscd_algorithm.h"
#include <set>
#include <map>
#include <cmath>
#include "graph.h"

#ifdef USE_HASH_MAP
#include <unordered_map>
#endif

//#include <chrono>
//#include <iostream>

namespace mscd {
namespace alg {

#ifdef USE_HASH_MAP
class pair_hash {
public:
    size_t operator()(const std::pair<long, long> & k) const {
        return k.first*101 + k.second;
    }
};
struct pair_equals : std::binary_function<const std::pair<long,long> &, const std::pair<long,long> &, bool> {
    result_type operator()(first_argument_type p1, second_argument_type p2) const {
        return ((p1.first == p2.first) && (p1.second == p2.second)) || ((p1.first == p2.second) && (p1.second == p2.first));
    }
}; 
#define simmap std::unordered_map<std::pair<long,long>,preal,pair_hash,pair_equals>
#else
#define simmap std::map<std::pair<long,long>,preal>
#endif
    
/** Multi-scale community detection using Huang, Sun, Liu, Song, and Weninger's method.
 *	This method assumes the network is undirected.
 */
class MSCD_HSLSW: public MSCDAlgorithm {
	
public:
	
	//typedef std::chrono::high_resolution_clock clock;
	//clock::time_point s0, i0, g0, c0, m0;
	//std::chrono::milliseconds dur_s, dur_i, dur_g, dur_c, dur_m;
	
	/// Default constructor
	MSCD_HSLSW() : merge_th(0.5) {}
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "HSLSW"; }
	
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
			if (params.size() > 1) {
				fprintf(stderr, "Too many arguments\n");
				return false;
			}
			if ((params[0] < 0.) && (params[0] > 1.)) {
				fprintf(stderr, "Parameter 1: Merging threshold must be within [0,1]\n Incorrect value %f\n", params[1]);
				return false;
			}
			this->SetMergeTh(params[0]);
			return true;
		}
		return false;
	}
	
	/// Set the merge threshold
	inline void SetMergeTh(const double value) { this->merge_th = value; }
	
	// Return the merge threshold
	inline double GetMergeTh() const { return this->merge_th; }
	
private:
	
	/// Merge threshold
	double merge_th;
	
    /// Bool vectors used to speed up community membership.
	/// (They are assigned to false for all bits and should always return to this state after use.)
	std::vector<bool> bitvector, bitvector2;
    
	/// Grow the given community
	bool grow(const ds::Graph::adjlist &, const simmap &, double, std::vector<long> &, double &, double &, std::set<long> &);
	
	/// Compute the similarities between adjacent nodes
	void computeSimilarities(const ds::Graph::adjlist &, simmap &) const;
	
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

#ifdef __DEBUG__
	void DbCheckSims(const ds::Graph::adjlist &, const simmap &, const std::vector<long> &, double &, double &) const;
#endif
	
};

} // namespace alg
} // namespace mscd

#endif
