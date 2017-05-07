/*
 *  mscd_afg.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 05/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mscd_afg.h"
#include "graph.h"
#include "communities.h"
#include <algorithm>
#include <map>

// Register algorithm
#include "registry.h"

#ifdef __DEBUG__
#include <cmath>
#endif

namespace mscd {
namespace alg {
	
static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_AFG);
	
bool MSCD_AFG::Run(const ds::Graph & g,
				   const std::vector<double> & ps,
				   std::vector<ds::Communities> & p_coms,
				   std::vector<double> & Qs,
				   const ds::Communities & init_coms) {
	
	// Get adjacency list
	const ds::Graph::adjlist & al = g.GetAdjList();
	
	// Resize result variables accordingly
	p_coms.resize(ps.size());
	Qs.resize(ps.size());
	
	// Initial number of communities
	long nb_init_coms = (init_coms.IsEmpty())?al.size():init_coms.GetNbCommunities();
	
	// Global work variables (used throughout a parameter loop)
	ds::Communities communities(nb_init_coms); // Current communities (groups of nodes)
	std::vector<std::vector<long> > & coms = communities.GetCommunityList();
	std::vector<preal> d(al.size(), 0.); // Strength vector (degree vector)
	std::vector<preal> tw(nb_init_coms), twi(nb_init_coms); // Total community weight and internal weight
	std::vector<long> ncom(al.size()), ncidx(al.size()); // node->community and community index tables
	double i2m; // i2m = 1/2m
	double Q; // Current Q value
	double last_r = 0.; // Last parameter value (r)
	
	// Initialise degree vector
	for (long i=0, j; i<al.size(); ++i)
		for (j=0; j<al[i].size(); ++j)
			d[i] += al[i][j].second;
	
	// Initialise partition and relevant variables
	if (init_coms.IsEmpty()) {
		for (long i=0; i<al.size(); ++i) {
			coms[i].resize(1); coms[i][0] = i;
			tw[i] = d[i];
			twi[i] = 0.;
		}
        ncidx.assign(ncidx.size(), 0);
	}
	else {
		coms = init_coms.GetCommunityList();
		for (long i=0, j; i<coms.size(); ++i) {
			tw[i] = twi[i] = 0.;
			for (j=0; j<coms[i].size(); ++j) {
				tw[i] +=  d[coms[i][j]];
				ncom[coms[i][j]] = i;
                ncidx[coms[i][j]] = j;
			}
		}
		for (long i=0, j; i<al.size(); ++i)
			for (j=0; j<al[i].size(); ++j)
				if (ncom[i] == ncom[al[i][j].first])
					twi[ncom[i]] += al[i][j].second;
	}
	
	// For each parameter value (noted r in the original paper)
	for (int p=0; p<ps.size(); ++p) {
		
		// Check that the parameter is not negative
		double rasymp = -g.GetTotalWeight()/(static_cast<double>(al.size()));
		if (ps[p] < rasymp) {
			fprintf(stderr, "Error: Parameter value p=%f cannot be below rasymp=%f\n", ps[p], rasymp);
			return false;
		}
		
		// Compute 1/2m
		i2m = 1./(g.GetTotalWeight()+(static_cast<double>(g.GetNbNodes()))*ps[p]);
		
		// Sets table: node -> community / Update community weights
		double dr = ps[p] - last_r;
		for (long i=0, j; i<coms.size(); ++i) {
			for (j=0; j<coms[i].size(); ++j) ncom[coms[i][j]] = i;
			tw[i] += dr*(static_cast<preal>(coms[i].size()));
			twi[i] += dr*(static_cast<preal>(coms[i].size()));
		}

		// Initial Q value of the current partition
		Q = this->computeQ(tw, twi, i2m);
		PrintDb("Initial Q for p=%f: %f (check: %f)\n", ps[p], Q, this->DbCheckQ(g, ncom, ps[p]));
		
		// Local work/optimisation variables (store local indices or results)
		std::vector<long> l, nbc; // List of nodes/communities to check, neighbour community list
		std::map<long,preal> nbc_wn; // Neighbour community connection weight (c->w)
		double dQ, best_dQ; // dQ and best_dQ values
		long n, c, cn, new_c; // Node and community values
		double tmp1, tmp2, tmp3; // Local results
		preal self_loops; // Self-loops value storage
		long i, j, k;
		
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
						if (nbc_wn.find(c) == nbc_wn.end()) {
							if (c != ncom[n]) nbc.push_back(c);
							nbc_wn[c] = 0.;
						}
						nbc_wn[c] += 2.*al[n][i].second;
						if (k==n) self_loops = 2.*al[n][i].second;
					}
					
					// For each neighbour community, check possible moves
					best_dQ = DQ_ZERO;
					c = ncom[n];
					tmp1 = -(nbc_wn[c]-self_loops); // Self-loops are added back to the other
					tmp2 = 2.*(d[n]+ps[p])*i2m;
					tmp3 = tw[c]-(d[n]+ps[p]);
					for (i=0; i<nbc.size(); ++i) {
						dQ = tmp1 + nbc_wn[nbc[i]] + (tmp2*(tmp3-tw[nbc[i]]));
						if (dQ > best_dQ) {
							best_dQ = dQ;
							new_c = nbc[i];
						}
					}
					
					// If a move is worth it, do it
					if (best_dQ > DQ_ZERO) {
						PrintDb("Moving %ld from %ld to %ld (bdQ=%f)\n", n+1, c+1, new_c+1, best_dQ);
                        // Update community's total weight
						tw[c] -= d[n] + ps[p];
						tw[new_c] += d[n] + ps[p];
                        // Update community's internal weight
						twi[c] -= nbc_wn[c] + ps[p];
						twi[new_c] += nbc_wn[new_c] + ps[p] + self_loops; // Add self loops
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
						preal eqQ = this->DbCheckQ(g, ncom, ps[p]);
						if (std::fabs(Q-eqQ) >= ERROR_Q_DIFF) {
							fprintf(stderr, "Warning: found Q=%f, should be Q=%f. Diff = %f\n", Q, eqQ, fabs(Q-eqQ));
							fprintf(stderr, "-> Correcting\n"); Q = eqQ;
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
						for (i=0; i<al[n].size(); ++i) {
							c = ncom[al[n][i].first];
							if (nbc_wn.find(c) == nbc_wn.end()) {
								if (c != cn) nbc.push_back(c);
								nbc_wn[c] = 0.;
							}
							nbc_wn[c] += 2.*al[n][i].second;
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
						preal eqQ = this->DbCheckQ(g, ncom, ps[p]);
						if (std::fabs(Q-eqQ) >= ERROR_Q_DIFF) {
							fprintf(stderr, "Warning: found Q=%f, should be Q=%f. Diff = %f\n", Q, eqQ, fabs(Q-eqQ));
							fprintf(stderr, "-> Correcting\n"); Q = eqQ;
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
		
		// Store last parameter
		last_r = ps[p];
		
	} // For each parameter
	
	// Everything went fine
	return true;
}

#ifdef __DEBUG__
double MSCD_AFG::DbCheckQ(const ds::Graph & g, const std::vector<long> & ncom, const double p) const {
	const ds::Graph::adjlist & al = g.GetAdjList();
	std::vector<preal> d(al.size());
	double m2 = g.GetTotalWeight() + (static_cast<double>(g.GetNbNodes()))*p;
	for (long i=0, j; i<al.size(); ++i) {
		d[i] = p;
		for (j=0; j<al[i].size(); ++j)
			d[i] += al[i][j].second;
	}
	double q1=0., q2 = 0.;
	for (long i=0, j; i<al.size(); ++i) {
		q1 += p;
		for (j=0; j<al.size(); ++j) {
			if (ncom[i] == ncom[j]) {
				std::vector<std::pair<long,preal> >::const_iterator it = al[i].begin();
				while ((it != al[i].end()) && (it->first != j)) ++it;
				if (it != al[i].end()) q1 += it->second;
				q2 += (d[i]*d[j])/m2;
			}
		}
	}
	return (q1 - q2)/m2;
}
#endif
	
} // namespace alg
} // namespace mscd
