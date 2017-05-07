/*
 *  mscd_som.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 08/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_SOM_H_
#define MSCD_SOM_H_

// Includes
#include "mscd_algorithm.h"
#include <map>
#include <queue>
#include "graph.h"

// Use the library Armadillo for matrix operations speed up
// (enable LAPACK and BLAS for full speed up)
#ifdef USE_ARMADILLO
#include "armadillo"
#define MAT_TYPE arma::mat
#else
#define MAT_TYPE preal
#endif

namespace mscd {
namespace alg {
	
/** Multi-scale community detection using Stability optimisation.
 *	This implementation is using a matrix representation for the Markov chain and an ajacency list
 *	for the rest of the computation. Therefore this implementation uses transformation matrices
 *	and matrices operations (multiplication).
 *	When Armadillo is not enabled, naive matrix multiplication is performed and a symmetric
 *	matrix is assumed in input in order to speed up the multiplication.
 *	The algorithm keeps the computed matrices in memory for potential use in further computation
 *	unless the memory saving mode is on in which case only the last matrix is kept. When
 *	computing matrices for successive exponent values (e.g. 1,2,3,...) this mode significantly
 *	reduces the memory usage without loss in performance.
 */
class MSCD_SOM: public MSCDAlgorithm {
	
public:
	
	/// Default constructor
	MSCD_SOM() : memory_saving(0) {}
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "SOM"; }
	
	/// Run the algorithm
	virtual bool Run(const ds::Graph &,
					 const std::vector<double> &,
					 std::vector<ds::Communities> &,
					 std::vector<double> &,
					 const ds::Communities &);
	
	/** Set extra parameters from a list of values
	 *	1. memory saving mode: 0 (none), positive for the number of graphs to keep
	 */
	virtual bool SetExtraParams(const std::vector<double> & params) {
		if (!params.empty()) {
			if (params.size() > 1) {
				fprintf(stderr, "Too many arguments\n");
				return false;
			}
			if (params[0] < 0.) {
				fprintf(stderr, "Parameter 1: Memory saving level can be 0 (off), or positive (number of graphs to keep in memory)\n  Incorrect value %f\n", params[1]);
				return false;
			}
			this->SetMemorySaving(params[0]);
			return true;
		}
		return false;
	}
	
	/// Set the memory saving level (number of graphs kept in memory)
	inline bool SetMemorySaving(const int value) {
		if (value < 0) return false;
		this->memory_saving = value;
		return true;
	}
	
	// Get the memory saving level
	inline int GetMemorySaving() const { return this->memory_saving; }
	
private:
	
	/// Memory saving level (O: OFF, >0: keep the given number of graphs in memory)
	int memory_saving;

	/// Check the number of graph kept in memory and delete as necessary
	inline void checkMemory(std::map<int,MAT_TYPE*> & Mts) const {
		if (this->memory_saving == 0) return;
		std::vector<int> exp;
		std::map<int,MAT_TYPE*>::reverse_iterator it;
		for (it = Mts.rbegin(); it != Mts.rend(); ++it)
			if (it->first != 1) exp.push_back(it->first);
		while (exp.size() > this->memory_saving) {
			if (this->verbose) fprintf(stdout, "Delete matrix for t=%d\n", exp.back());
#ifndef USE_ARMADILLO
			delete[] Mts[exp.back()];
#else
			delete Mts[exp.back()];
#endif
			Mts.erase(exp.back());
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
	void setCurrentAL(const double, const ds::Graph::adjlist &, const std::vector<preal> &, std::map<int,MAT_TYPE*> &, ds::Graph::adjlist &);

#ifndef USE_ARMADILLO
	/// Multiply transition matrices mat1 by mat2 and stores the result in mat
	inline void mult(preal const * mat1, preal const * mat2, preal * mat, const std::vector<preal> & d) const {
		preal dc;
		for (long r=0, c, i, n=d.size(); r<n; ++r) {
			for (c=0; c<n; ++c) {
				dc = d[c];
				mat[r*n+c] = 0.;
				for (i=0; i<n; ++i) mat[r*n+c] += mat1[r*n+i]*(dc*mat2[c*n+i])/d[i];
			}
		}
	}
#endif

	/// Return the required transition matrix and create it if needed
	MAT_TYPE const * getMt(const ds::Graph::adjlist &, const int, const std::vector<preal> &, std::map<int,MAT_TYPE*> &) const;

#ifdef __DEBUG__
	double DbCheckQ(const ds::Graph::adjlist &, const std::vector<long> &, const double) const;
#endif

};

} // namespace alg
} // namespace mscd

#endif

/*
 /// Multiply m1 by m2 and stores the result in m (naive multiplication)
 inline void mult(preal const * mat1, preal const * mat2, preal * mat, const std::vector<preal> & d) const {
 int i, r, c, n=d.size();
 for (r=0; r<n; ++r) {
 for (c=0; c<n; ++c) {
 mat[r*n+c] = 0.;
 for (i=0; i<n; ++i) mat[r*n+c] += mat1[r*n+i]*mat2[i*n+c];
 }
 }
 }*/

/*
 // naive matrix multiplication using pointers
 preal const * m1 = mat1, * m2 = mat2;
 preal * m = mat;
 for (int r=0; r<n; ++r) {
 m1 = mat1 + r*n;
 for (c=0; c<n; ++c) {
 m2 = mat2 + c;
 for (i=0; i<n; ++i) {
 *m += *m1++ + *m2;
 m2 += n;
 }
 ++m;
 }
 }
 */

/*
 // opt mult using pointers
preal const * m1 = mat1, * m2 = mat2;
preal * m = mat, dc;
std::vector<preal>::const_iterator di;
for (int r=0,c,i,n=d.size(); r<n; ++r) {
	for (c=0; c<n; ++c) {
		m1 = mat1 + r*n;
		m2 = mat2 + c*n;
		dc = d[c];
		di = d.begin();
		for (i=0; i<n; ++i)
			*m += (*m1++) * (((*m2++)*dc)/(*di++));
			++m;
	}
}
*/