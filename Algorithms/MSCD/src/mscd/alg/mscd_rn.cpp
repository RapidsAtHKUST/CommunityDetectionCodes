/*
 *  mscd_rn.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 06/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mscd_rn.h"
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
	
static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_RN);
	
bool MSCD_RN::Run(const ds::Graph & g,
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
	std::vector<preal> twi(nb_init_coms); // Total community internal weight
	std::vector<long> ncom(al.size()), ncidx(al.size()); // node->community and community index tables
	double Q; // Current Q value
	
	// Initialise partition and relevant variables
	if (init_coms.IsEmpty()) {
		for (long i=0; i<al.size(); ++i) {
			coms[i].resize(1); coms[i][0] = i;
			twi[i] = 0.;
		}
        ncidx.assign(ncidx.size(), 0);
	}
	else {
		coms = init_coms.GetCommunityList();
		for (long i=0, j; i<coms.size(); ++i) {
			twi[i] = 0.;
			for (j=0; j<coms[i].size(); ++j) {
				ncom[coms[i][j]] = i;
                ncidx[coms[i][j]] = j;
            }
		}
		for (long i=0, j; i<al.size(); ++i)
			for (j=0; j<al[i].size(); ++j)
				if (ncom[i] == ncom[al[i][j].first])
					twi[ncom[i]] += al[i][j].second;
	}
	
	// For each parameter (noted gamma in the original paper)
	for (int p=0; p<ps.size(); ++p) {

		// Check that the parameter is not negative
		if (ps[p] < 0.) {
			fprintf(stderr, "Error: Parameter value p=%f must be positive\n", ps[p]);
			return false;
		}
		
		// Sets table: node -> community
		for (long i=0, j; i<coms.size(); ++i)
			for (j=0; j<coms[i].size(); ++j)
				ncom[coms[i][j]] = i;
		
		// Initial Q value of the current partition
		Q = this->computeQ(twi, coms, ps[p]);
		PrintDb("Initial Q for p=%f: %f (check: %f)\n", ps[p], Q, this->DbCheckQ(g, ncom, ps[p]));
		
		// Local work/optimisation variables (store local indices or results)
		std::vector<long> l, nbc; // List of nodes/communities to check, neighbour community list
		std::map<long,preal> nbc_wn; // Neighbour community connection weight (c->w)
		double dQ, best_dQ; // dQ and best_dQ values
		long n, c, cn, new_c; // Node and community values
		double tmp1; // Local results
		long i, j; // Loop indices
		
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
					
					// Find neighbour communities of n and its connection stength to them
					nbc.clear();
					nbc_wn.clear();
					for (i=0; i<al[n].size(); ++i) {
						c = ncom[al[n][i].first];
						if (nbc_wn.find(c) == nbc_wn.end()) {
							if (c != ncom[n]) nbc.push_back(c);
							nbc_wn[c] = 0.;
						}
						++nbc_wn[c];
					}
					
					// For each neighbour community, check possible moves
					best_dQ = DQ_ZERO;
					c = ncom[n];
					tmp1 = -nbc_wn[c] + ps[p]*(static_cast<double>(coms[c].size())-1.-nbc_wn[c]);
					for (i=0; i<nbc.size(); ++i) {
						dQ = tmp1 + nbc_wn[nbc[i]] - ps[p]*(static_cast<double>(coms[nbc[i]].size())-nbc_wn[nbc[i]]);
						if (dQ > best_dQ) {
							best_dQ = dQ;
							new_c = nbc[i];
						}
					}
					
					// If a move is worth it, do it
					if (best_dQ > DQ_ZERO) {
						PrintDb("Moving %ld from %ld to %ld (bdQ=%f)\n", n+1, c+1, new_c+1, best_dQ);
						// Update community's internal weight
                        twi[c] -= nbc_wn[c];
						twi[new_c] += nbc_wn[new_c];
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
						Q += best_dQ;
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
							++nbc_wn[c];
						}
					}
					
					// For each neighbour community, check possible moves
					best_dQ = DQ_ZERO;
					for (i=0; i<nbc.size(); ++i) {
						c = nbc[i];
						dQ = nbc_wn[c] - ps[p]*(static_cast<double>(coms[cn].size()*coms[c].size()) - nbc_wn[c]);
						if (dQ > best_dQ) {
							best_dQ = dQ;
							new_c = nbc[i];
						}
					}
					
					// If a move is worth it, do it
					if (best_dQ > DQ_ZERO) {
						PrintDb("Merging %ld and %ld (bdQ=%f)\n", cn+1, new_c+1, best_dQ);
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
						Q += best_dQ;
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
				twi[i] = twi[j];
				++i; ++j;
			}
		}
		if (i < coms.size()) {
			coms.resize(i);
			twi.resize(i);
		}
		
		// Store final partition for the current parameter
		this->storeCommunities(communities, p_coms[p]);
		Qs[p] = -Q;
		
	} // For each parameter
	
	// Everything went fine
	return true;
}

#ifdef __DEBUG__
double MSCD_RN::DbCheckQ(const ds::Graph & g, const std::vector<long> & ncom, const double p) const {
	const ds::Graph::adjlist & al = g.GetAdjList();
	std::map<long,double> n, w;
	for (long i=0, j; i<al.size(); ++i) {
		if (n.find(ncom[i]) == n.end()) n[ncom[i]] = 0;
		++n[ncom[i]];
		for (j=0; j<al[i].size(); ++j) {
			if (ncom[i] == ncom[al[i][j].first]) {
				if (w.find(ncom[i]) == w.end()) w[ncom[i]] = 0;
				++w[ncom[i]];
			}
		}
	}
	double Q = 0.;
	for (std::map<long,double>::const_iterator it = n.begin(); it != n.end(); ++it)
		Q += w[it->first]*0.5 - p*(it->second*(it->second-1)-w[it->first])*0.5;
	return Q;
}
#endif
	
} // namespace alg
} // namespace mscd
