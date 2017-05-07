/*
 *  mscd_som.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 08/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mscd_som.h"
#include "graph.h"
#include "communities.h"
#include <algorithm>
#include <map>
#include <cmath>

// Register algorithm
#include "registry.h"

namespace mscd {
namespace alg {
	
static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_SOM);

bool MSCD_SOM::Run(const ds::Graph & g,
				   const std::vector<double> & ps,
				   std::vector<ds::Communities> & p_coms,
				   std::vector<double> & Qs,
				   const ds::Communities & init_coms) {
	
	// Get adjacency list
	const ds::Graph::adjlist & gal = g.GetAdjList();
	
	// Resize result variables accordingly
	p_coms.resize(ps.size());
	Qs.resize(ps.size());
	
	// Initial number of communities
	long nb_init_coms = (init_coms.IsEmpty())?gal.size():init_coms.GetNbCommunities();
	
	// Global work variables (used throughout a parameter loop)
	ds::Communities communities(nb_init_coms); // Current communities (groups of nodes)
	std::vector<std::vector<long> > & coms = communities.GetCommunityList();
	std::vector<preal> d(gal.size(), 0.); // Strength vector (degree vector)
	std::vector<preal> tw(nb_init_coms), twi(nb_init_coms); // Total community weight and longernal weight
	std::vector<long> ncom(gal.size()), ncidx(gal.size()); // node->community and community index tables
	ds::Graph::adjlist al; // Current adjacency list
	double i2m = 1./g.GetTotalWeight(); // i2m = 1/2m
	double Q; // Current Q value
	std::map<int,MAT_TYPE*> Mts; // Time transition matrices

	// Initialise degree vector
	for (long i=0, j; i<gal.size(); ++i)
		for (j=0; j<gal[i].size(); ++j) d[i] += gal[i][j].second;
	
	// Initialise partition and relevant variables
	if (init_coms.IsEmpty()) {
		for (long i=0; i<gal.size(); ++i) {
			coms[i].resize(1); coms[i][0] = i;
			tw[i] = d[i];
		}
        ncidx.assign(ncidx.size(), 0);
	}
	else {
		coms = init_coms.GetCommunityList();
		for (long i=0, j; i<coms.size(); ++i) {
			tw[i] = 0.;
			for (j=0; j<coms[i].size(); ++j) {
                tw[i] += d[coms[i][j]];
                ncidx[coms[i][j]] = j;
            }
		}
	}
	
	// For each parameter (noted t in the original paper)
	for (int p=0; p<ps.size(); ++p) {
		
		// Check that the parameter is not negative
		if (ps[p] < 0.) {
			fprintf(stderr, "Error: Parameter value p=%f must be positive\n", ps[p]);
			return false;
		}

		// Set the list to the appropriate values
		this->setCurrentAL(ps[p], gal, d, Mts, al);

		// Sets table: node -> community
		for (long i=0, j; i<coms.size(); ++i)
			for (j=0; j<coms[i].size(); ++j)
				ncom[coms[i][j]] = i;
		
		// Recompute internal weights
		for (long i=0, j, k, n; i<coms.size(); ++i) {
			twi[i] = 0.;
			for (j=0; j<coms[i].size(); ++j) {
				n = coms[i][j];
				for (k=0; k<al[n].size(); ++k)
					if (ncom[al[n][k].first] == i)
						twi[i] += al[n][k].second;
			}
		}

		// Initial Q value of the current partition
		Q = this->computeQ(tw, twi, i2m);
		PrintDb("Initial Q for p=%f: %f (real: %f)\n", ps[p], Q, this->DbCheckQ(al, ncom, i2m));
		
		// Display information on current adjacency list
		if (this->verbose) {
			long nb_edges = 0;
			preal emin = std::numeric_limits<preal>::max(), emax = 0., eave = 0.;
			for (long i=0, j; i<al.size(); ++i) {
				nb_edges += al[i].size();
				for (j=0; j<al[i].size(); ++j) {
					if (al[i][j].first == i) ++nb_edges;
					if (al[i][j].second < emin) emin = al[i][j].second;
					if (al[i][j].second > emax) emax = al[i][j].second;
					eave += al[i][j].second;
				}
			}
			eave /= static_cast<preal>(nb_edges);
			if (!g.IsDirected()) nb_edges /= 2;
			//fprintf(stdout, "T=%f\t%d edges\tmin=%f\tmax=%f\tave=%f\n", ps[p], nb_edges, emin, emax, eave);
		}
		
		// Local work/optimisation variables used within embedded loops (store local indices or results)
		std::vector<long> l, nbc; // List of nodes/communities to check, neighbour community list
		std::map<long,preal> nbc_wn; // Neighbour community connection weight (c->w)
		double dQ, best_dQ; // dQ and best_dQ values
		long n, c, cn, new_c; // Node and community values
		double tmp1, tmp2, tmp3; // Local results
		preal self_loops;
		long i, j, k; // Loop indices
		
		// While changes can potentially be made to improve Q
		bool check_nodes = true, check_communities = true, moved;
		while (check_nodes) {

			// While nodes can potentially be moved
			do {
				PrintDb("Node loop\n");
				moved = false;

				// List of nodes to inspect
				l.resize(al.size());
				for (i=0; i<l.size(); ++i) l[i] = i;
				
				// While there are nodes to inspect
				while (!l.empty()) {
					
					// Pick a node at random and remove it from the node list
					i = rand() % l.size();
					n = l[i];
					l[i] = l.back();
					l.pop_back();
					
					// Find neighbour communities of n and its connection strength to them
					nbc.clear();
					nbc_wn.clear();
					self_loops = 0.;
					for (i=0; i<al[n].size(); ++i) {
						k = al[n][i].first;
						c = ncom[k];
						if (nbc_wn.find(c) == nbc_wn.end()) nbc_wn[c] = 0.;
						nbc_wn[c] += 2.*al[n][i].second;
						if (k==n) self_loops = 2.*al[n][i].second;
					}
					for (i=0; i<gal[n].size(); ++i) {
						c = ncom[gal[n][i].first];
						if ((c != ncom[n]) && (std::find(nbc.begin(), nbc.end(), c) == nbc.end()) && (nbc_wn.find(c) != nbc_wn.end()))
							nbc.push_back(c);
					}

					// For each neighbour community, check possible moves
					best_dQ = DQ_ZERO;
					c = ncom[n];
					tmp1 = -(nbc_wn[c]-self_loops); // Self-loops are added back to the other
					tmp2 = 2.*d[n]*i2m;
					tmp3 = tw[c]-d[n];
					for (i=0; i<nbc.size(); ++i) {
						dQ = tmp1 + nbc_wn[nbc[i]] + (tmp2*(tmp3-tw[nbc[i]]));
						if ((dQ > best_dQ) && (this->isStillConnected(gal, coms[c], ncom, n))) {
							best_dQ = dQ;
							new_c = nbc[i];
						}
					}
					
					// If a move is worth it, do it
					if (best_dQ > DQ_ZERO) {
						PrintDb("Moving %ld from %ld to %ld (bdQ=%f)\n", n+1, c+1, new_c+1, best_dQ);
                        // Update community's total weight
						tw[c] -= d[n];
						tw[new_c] += d[n];
                        // Update community's internal weight
						twi[c] -= nbc_wn[c];
						twi[new_c] += nbc_wn[new_c] + self_loops; // Add self loops
                        // Update node-community table
						ncom[n] = new_c;
						// Remove node and update community index table
                        coms[c][ncidx[n]] = coms[c].back();
                        ncidx[coms[c].back()] = ncidx[n];
                        coms[c].pop_back();
                        // Add node and update community index table
                        ncidx[n] = coms[new_c].size();
						coms[new_c].push_back(n);
                        // Update Q
						Q += best_dQ * i2m;
                        // Update switches
						check_communities = moved = true;
#ifdef __DEBUG__
						preal realQ = this->DbCheckQ(al, ncom, i2m);
						preal rcQ = this->computeQ(tw, twi, i2m);
						if (std::fabs(Q-realQ) >= ERROR_Q_DIFF) {
							fprintf(stderr, "Warning: found Q=%f, recomputed Q=%f - preal Q=%f. Diff(found,real) = %f\n", Q, rcQ, realQ, fabs(Q-realQ));
							fprintf(stderr, "-> Correcting\n"); Q = realQ;
						}
#endif
					}
					
				} // for all nodes in the list
			}
			while (moved); // While nodes can potentially be moved
			// Nodes need not be checked again unless communites are merged
			check_nodes = false;
			
			// If no need to check communities, jump the community section
			if (!check_communities) break;
			
			// While communities can potentially be merged
			do {
				PrintDb("Community loop\n");
				moved = false;
				
				// List of communities to inspect
				l.resize(coms.size());
				for (i=0; i<l.size(); ++i) l[i] = i;
				
				// While there are communities to inspect
				while (!l.empty()) {
					
					// Pick a community at random and remove it from the community list
					i = rand() % l.size();
					cn = l[i];
					l[i] = l.back();
					l.pop_back();
					
					// Find neighbour communities of c and its connection stength to them
					nbc.clear();
					nbc_wn.clear();
					for (j=0; j<coms[cn].size(); ++j) {
						n = coms[cn][j];
						// Set connection strength with current edges
						for (i=0; i<al[n].size(); ++i) {
							c = ncom[al[n][i].first];
							if (nbc_wn.find(c) == nbc_wn.end()) nbc_wn[c] = 0.;
							nbc_wn[c] += 2.*al[n][i].second;
						}
						// Create the list of neighbour communities using the original edges
						for (i=0; i<gal[n].size(); ++i) {
							c = ncom[gal[n][i].first];
							if ((c != cn) && (std::find(nbc.begin(), nbc.end(), c) == nbc.end()) && (nbc_wn.find(c) != nbc_wn.end()))
								nbc.push_back(c);
						}
					}
					
					// For each neighbour community, check possible moves
					best_dQ = DQ_ZERO;
					tmp1 = 2.*i2m*tw[cn];
					for (i=0; i<nbc.size(); ++i) {
						c = nbc[i];
						dQ = nbc_wn[c] - tmp1*tw[c];
						if (dQ > best_dQ) {
							best_dQ = dQ;
							new_c = nbc[i];
						}
					}
					
					// If a move is worth it, do it
					if (best_dQ > DQ_ZERO) {
						PrintDb("Merging %ld and %ld (bdQ=%f)\n", cn+1, new_c+1, best_dQ);
                        // Update community's total weight
						tw[new_c] += tw[cn];
						tw[cn] = 0.;
                        // Update community's internal weight
						twi[new_c] += twi[cn] + nbc_wn[new_c];
						twi[cn]  = 0.;
                        // Merge
						for (i=0; i<coms[cn].size(); ++i) {
							// Update node-community table
							ncom[coms[cn][i]] = new_c;
                            // Update community index table
                            ncidx[coms[cn][i]] = coms[new_c].size();
                            // Add node
							coms[new_c].push_back(coms[cn][i]);

						}
                        // Clear former community that got added to the other
						coms[cn].clear();
                        // Update Q
						Q += best_dQ * i2m;
                        // Update switches
						check_nodes = moved = true;
#ifdef __DEBUG__
						preal realQ = this->DbCheckQ(al, ncom, i2m);
						preal rcQ = this->computeQ(tw, twi, i2m);
						if (std::fabs(Q-realQ) >= ERROR_Q_DIFF) {
							fprintf(stderr, "Warning: found Q=%f, recomputed Q=%f - preal Q=%f. Diff(found,real) = %f\n", Q, rcQ, realQ, fabs(Q-realQ));
							fprintf(stderr, "-> Correcting\n"); Q = realQ;
						}
#endif
					}
					
				} // for all communities in the list
			}
			while(moved); // While communities can potentially be merged
			// Communities need not be checked again unless nodes are moved
			check_communities = false;
			
		} // While changes can potentially be made

		// Re-index communities
		i=0; j=0;
		while (j<coms.size()) {
			while ((i<coms.size()) && (!coms[i].empty())) ++i;
			if (j<=i) j=i+1;
			while ((j<coms.size()) && (coms[j].empty())) ++j;
			if (j<coms.size()) {
				coms[i].swap(coms[j]);
				tw[i] = tw[j];
				twi[i] = twi[j];
				++i; ++j;
			}
		}
		if (i < coms.size()) {
			coms.resize(i);
			tw.resize(i);
			twi.resize(i);
		}

		// Store final partition for the current parameter
		this->storeCommunities(communities, p_coms[p]);
		Qs[p] = Q;
		
	} // For each parameter
	
	// Clear Mt

	for (std::map<int,MAT_TYPE*>::iterator it = Mts.begin(); it != Mts.end(); ++it)
#ifndef USE_ARMADILLO
		delete[] it->second;
#else
		delete it->second;
#endif
	// Everything went fine
	return true;
}

void MSCD_SOM::setCurrentAL(const double t, const ds::Graph::adjlist & al1, const std::vector<preal> & d, std::map<int,MAT_TYPE*> & Mts, ds::Graph::adjlist & al) {
	// Compute the current transitions
	if (t == 0.) {
		al.resize(al1.size());
		for (long i=0; i<al1.size(); ++i) {
			al[i].clear();
			al[i].push_back(std::make_pair(i,d[i]));
		}
	}
	else if (t == 1.) al = al1;
	else {
		al.resize(al1.size());
		// Parameter is an integer
		if (std::floor(t) == t) {
#ifndef USE_ARMADILLO
			const preal * mat = this->getMt(al1, t, d, Mts);
#else
			const arma::mat & mat = *this->getMt(al1, t, d, Mts);
#endif
			long n = al1.size();
			for (long i=0, j; i<n; ++i) {
				al[i].clear();
				for (j=0; j<n; ++j)
#ifndef USE_ARMADILLO
					if (mat[i*n+j] != 0.) al[i].push_back(std::make_pair(j,d[i]*mat[i*n+j]));
#else
					if (mat(i,j) != 0.)	al[i].push_back(std::make_pair(j,d[i]*mat(i,j)));
#endif
			}
		}
		// Parameter is a preal value: Mt is computed by interpolation
		else {
			double ft = std::floor(t), dt = t-ft;
			preal v;
            // When interpolation is between 0 and 1, no need to compute matrices
			if (ft == 0.) {
                bool selfloop;
				for (long i=0, j; i<al1.size(); ++i) {
					al[i].clear();
					selfloop = false;
					for (j=0; j<al1[i].size(); ++j) {
						if ((!selfloop) && (al1[i][j].first > i)) {
							al[i].push_back(std::make_pair(i,(1.-dt)*d[i]));
							selfloop = true;
						}
						if (al1[i][j].first == i)
							al[i].push_back(std::make_pair(i,(1.-dt)*d[i] + dt*al1[i][j].second));
						else
							al[i].push_back(std::make_pair(al1[i][j].first, dt*al1[i][j].second));
					}
					if (!selfloop)
						al[i].push_back(std::make_pair(i,(1.-dt)*d[i]));
				}
			}
			else {
				// Ensure that the memory mode will temporarily keep both graphs in memory
				bool inc_memory = false;
				if ((ft > 1) && (this->memory_saving == 1)) {
					this->memory_saving = 2;
					inc_memory = true;
				}
				// Get graphs
#ifndef USE_ARMADILLO
				const preal	* mat1 = this->getMt(al1, ft, d, Mts),
							* mat2 = this->getMt(al1, ft+1., d, Mts);
#else
				const arma::mat & mat1 = *this->getMt(al1, ft, d, Mts),
								& mat2 = *this->getMt(al1, ft+1., d, Mts);
#endif				
				long n = al1.size();
				for (long i=0, j; i<n; ++i) {
					al[i].clear();
					for (j=0; j<n; ++j) {
#ifndef USE_ARMADILLO
						v = (1.-dt)*mat1[i*n+j] + dt*mat2[i*n+j];
#else
						v = (1.-dt)*mat1(i,j) + dt*mat2(i,j);
#endif
						if (v != 0.) al[i].push_back(std::make_pair(j,d[i]*v));
					}
				}
				// Put back memory level if needed
				if (inc_memory) this->memory_saving = 1;
			}
		}
	}
}	

MAT_TYPE const * MSCD_SOM::getMt(const ds::Graph::adjlist & al, const int t, const std::vector<preal> & d, std::map<int,MAT_TYPE*> & Mts) const {
	// If M exists, return it
	if (Mts.find(t) != Mts.end()) return Mts[t];
	// T=1: out weights divided by the node strength 
	if (Mts.find(1) == Mts.end()) {
		PrintDb("t=%d: creating 1\n", t);
		long n = al.size();
#ifndef USE_ARMADILLO	
		preal * mat = new preal[n*n];
		for (long i=0; i<n*n; ++i) mat[i] = 0.;
		Mts[1] = mat;
#else
		Mts[1] = new arma::mat(al.size(),al.size());
		Mts[1]->zeros();
		arma::mat & mat = *Mts[1];
#endif
		for (long i=0, j; i<n; ++i) {
			for (j=0; j<al[i].size(); ++j)
#ifndef USE_ARMADILLO
				mat[i*n+al[i][j].first] = al[i][j].second/d[i];
#else
				mat(i,al[i][j].first) = al[i][j].second/d[i];
#endif
		}
	}
	if (t > 1) {
		// Create list of keys
		std::vector<int> exp;
		std::map<int,MAT_TYPE*>::const_iterator it;
		for (it = Mts.begin(); it != Mts.end(); ++it)
			exp.push_back(it->first);
		// Decompose using larger to smaller pre-computed exponents
		std::vector<int> ops;
		int i = exp.size() - 1, tleft = t;
		while (tleft > 0) {
			while (exp[i] > tleft) --i;
			ops.push_back(exp[i]);
			tleft -= exp[i];
			if (ops.size() > 1) {
				exp.push_back(t-tleft);
				i = exp.size() - 1;
			}
		}
		// Create Mt and keep intermediate Mt for future computations
		int prev = ops.front();
		ops.erase(ops.begin());
		while (!ops.empty()) {
			if (this->verbose)
				fprintf(stdout, "Computing %d from %d and %d\n", prev+ops.front(), prev, ops.front());
			if (Mts.find(prev) == Mts.end()) { // should not happen
				fprintf(stderr, "Error: Computation requires to keep additional matrices in memory\n-> Increase memory level\n");
				exit(EXIT_FAILURE);
			}
			// Compute new matrix
#ifndef USE_ARMADILLO
			preal * mat = new preal[al.size()*al.size()];
#else
			arma::mat * mat = new arma::mat(al.size(),al.size());
#endif
			Mts[prev+ops.front()] = mat;
#ifndef USE_ARMADILLO		
			MSCD_SOM::mult(Mts[prev], Mts[ops.front()], mat, d);
#else
			*mat = (*Mts[prev])*(*Mts[ops.front()]);
#endif
			// Prepare for next operation
			prev = prev + ops.front();
			ops.erase(ops.begin());
			// Check memory
			this->checkMemory(Mts);
		}
	}
	return Mts[t];
}	

#ifdef __DEBUG__
double MSCD_SOM::DbCheckQ(const ds::Graph::adjlist & al, const std::vector<long> & ncom, const double i2m) const {
	std::vector<preal> d(al.size());
	for (long i=0, j; i<al.size(); ++i) {
		d[i] = 0.;
		for (j=0; j<al[i].size(); ++j)
			d[i] += al[i][j].second;
	}
	double q1=0., q2 = 0.;
	for (long i=0, j; i<al.size(); ++i) {
		for (j=0; j<al.size(); ++j) {
			if (ncom[i] == ncom[j]) {
				std::vector<std::pair<long,preal> >::const_iterator it = al[i].begin();
				while ((it != al[i].end()) && (it->first != j)) ++it;
				if (it != al[i].end()) q1 += it->second;
				q2 += (d[i]*d[j])*i2m;
			}
		}
	}
	return (q1 - q2)*i2m;
}
#endif
	
} // namespace alg
} // namespace mscd