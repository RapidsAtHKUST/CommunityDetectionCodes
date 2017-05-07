/*
 *  mscd_so.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 15/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_SO_H_
#define MSCD_SO_H_

// Includes
#include "mscd_algorithm.h"
#include <map>
#include <queue>
#include "graph.h"

namespace mscd {
namespace alg {

/** Multi-scale community detection using Stability optimisation.
 *	This implementation is only using adjacency lists. This enables fast computation on sparse
 *	graphs. An edge threshold value can be set to discard edges below this value, thus helping
 *	to reduce the amount of edges and hence the amount of memory used by the program.
 *	The algorithm keeps the computed adjacency lists in memory for potential use in further
 *	computation unless the memory saving mode is on in which case only the last adjacency list
 *	is kept. When computing adjacency lists for successive exponent values (e.g. 1,2,3,...) this
 *	mode significantly reduces the memory usage without loss in performance.
 */
class MSCD_SO: public MSCDAlgorithm {

public:
	
	/// Default constructor
	MSCD_SO() : min_edge(0.), memory_saving(0)
	{}
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "SO"; }
	
	/// Run the algorithm
	virtual bool Run(const ds::Graph &,
					 const std::vector<double> &,
					 std::vector<ds::Communities> &,
					 std::vector<double> &,
					 const ds::Communities &);
	
	/** Set extra parameters from a list of values
	 *	1. minimum edge value: Must be positive
	 *	2. memory saving mode: 0 (none), positive for the number of graphs to keep
	 *	3. number of threads
	 */
	virtual bool SetExtraParams(const std::vector<double> & params) {
		if (!params.empty()) {
			if (params.size() > 2) {
				fprintf(stderr, "Too many arguments\n");
				return false;
			}
			if ((params.size() >= 1) && (params[0] < 0)) {
				fprintf(stderr, "Parameter 1: Edge threshold must be positive\n  Incorrect value %f\n", params[0]);
				return false;
			}
			if ((params.size() >= 2) && (params[1] < 0.)) {
				fprintf(stderr, "Parameter 2: Memory saving level can be 0 (off), or positive (number of graphs to keep in memory)\n  Incorrect value %f\n", params[1]);
				return false;
			}
			this->SetMinEdge(static_cast<preal>(params[0]));
			if (params.size() >= 2) this->SetMemorySaving(params[1]);
			return true;
		}
		return false;
	}
	
	/// Set the minimum edge value
	inline void SetMinEdge(const preal value) { this->min_edge = value; }
	
	// Return the minimum edge value
	inline preal GetMinEdge() const { return this->min_edge; }
	
	/// Set the memory saving level (number of graphs kept in memory)
	inline bool SetMemorySaving(const int value) {
		if (value < 0) return false;
		this->memory_saving = value;
		return true;
	}
	
	// Get the memory saving level
	inline int GetMemorySaving() const { return this->memory_saving; }

private:
	
	/// Minimum value an edge must carry to exist
	preal min_edge;
	
	/// Memory saving level (O: OFF, >0: keep the given number of graphs in memory)
	int memory_saving;

#ifdef USE_MULTITHREADING
	/// Number of threads allowed to run in parallel
	int nb_threads;
#endif
	
	/// Check the number of graph kept in memory and delete as necessary
	inline void checkMemory(std::map<int,ds::Graph::adjlist*> & alts) const {
		if (this->memory_saving == 0) return;
		std::vector<int> exp;
		std::map<int,ds::Graph::adjlist*>::reverse_iterator it;
		for (it = alts.rbegin(); it != alts.rend(); ++it)
			exp.push_back(it->first);
		while (exp.size() > this->memory_saving) {
			if (this->verbose) fprintf(stdout, "Delete adjacency list for t=%d\n", exp.back());
			delete alts[exp.back()];
			alts.erase(exp.back());
			exp.pop_back();
		}
	}
	
	/// Compute the criterion value
	inline double computeQ(const std::vector<preal> & tw,
						   const std::vector<preal> & twi,
						   const double i2m) const {
		double Q = 0.;
		for (long j=0; j<tw.size(); ++j)
			Q += twi[j] - tw[j]*tw[j]*i2m;
		return Q*i2m;
	}
	
	/// Return true if the given community component is not broken by the removal of the given node
	inline bool isStillConnected(const ds::Graph::adjlist & al, const std::vector<long> & com, const std::vector<long> & ncom, const long n) const {
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
				if ((ncom[nb] == ncom[n]) && (!visited[nb])) {
					visited[nb] = true;
					queue.push(nb);
					++n_visited;
				}
			}
		}
		//printf("%d %d\n", n_visited, com.size());
		return (n_visited==com.size()-1);
	}
	
	/// Set the current adjacency list to the appropriate values
	void setCurrentAL(const double, const ds::Graph::adjlist &, const std::vector<preal> &, std::map<int,ds::Graph::adjlist*> &, ds::Graph::adjlist &);
	
	/// Return the required adjacency list at a discrete time and create it if needed
	const ds::Graph::adjlist & getALt(const ds::Graph::adjlist &, const int, const std::vector<preal> &, std::map<int,ds::Graph::adjlist*> &) const;
	
	// Build a new adjacency list for time t1+t2 from two existing ones at times t1 and t2 
	void buildNextAL(const ds::Graph::adjlist &, const ds::Graph::adjlist &, const std::vector<preal> &, ds::Graph::adjlist &) const;
    
#ifdef __DEBUG__
	double DbCheckQ(const ds::Graph::adjlist &, const std::vector<long> &, const double) const;
#endif
	
};
	
} // namespace alg
} // namespace mscd

#endif
