/*
 *  mscd_so.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 15/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mscd_so.h"
#include "graph.h"
#include "communities.h"
#include <algorithm>
#include <map>
#include <cmath>
#include <limits>

#ifdef USE_MULTITHREADING
#include <thread>
#endif

// Register algorithm
#include "registry.h"

namespace mscd {
namespace alg {
	
static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_SO);
	
bool MSCD_SO::Run(const ds::Graph & g,
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
	std::vector<preal> tw(nb_init_coms), twi(nb_init_coms); // Total community weight and internal weight
	std::vector<long> ncom(gal.size()), ncidx(gal.size()); // node->community and community index tables
	ds::Graph::adjlist al; // Current adjacency list
	double i2m = 1./g.GetTotalWeight(); // i2m = 1/2m
	double Q; // Current Q value
	std::map<int,ds::Graph::adjlist*> alts; // Edgelists kept in memory
	
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
	
	// For each parameter
	for (int p=0; p<ps.size(); ++p) {
		
		// Check that the parameter is not negative
		if (ps[p] < 0.) {
			fprintf(stderr, "Error: Parameter value p=%f must be positive\n", ps[p]);
			return false;
		}
		
		// Set the edge list to the appropriate values
		this->setCurrentAL(ps[p], gal, d, alts, al);
		
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
					//if (al[i][j].first == i) {
						//++nb_edges;
						//eave += al[i][j].second;
					//}
					if (al[i][j].second < emin) emin = al[i][j].second;
					if (al[i][j].second > emax) emax = al[i][j].second;
					eave += al[i][j].second;
				}
			}
			eave /= static_cast<preal>(nb_edges);
			//if (!g.IsDirected()) nb_edges /= 2;
			//fprintf(stdout, "T=%f\t%d edges\tmin=%f\tmax=%f\tave=%f\n", ps[p], nb_edges, emin, emax, eave);
		}
		
		// Local work/optimisation variables for the following loops
		std::vector<long> l, nbc; // List of nodes/communities to check, neighbour community list
		std::map<long,preal> nbc_wn; // Neighbour community connection weight (c->w)
		double dQ, best_dQ; // dQ and best_dQ values
		long n, c, cn, new_c; // Node and community values
		double tmp1, tmp2, tmp3; // Local results
		preal self_loops;
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
						//printf("dq=%f\n", dQ*i2m);
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
	
	// Everything went fine
	return true;
}

void MSCD_SO::setCurrentAL(const double t, const ds::Graph::adjlist & al1, const std::vector<preal> & d, std::map<int,ds::Graph::adjlist*> & alts, ds::Graph::adjlist & al) {
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
		// Parameter is an integer
		if (std::floor(t) == t) al = this->getALt(al1, t, d, alts);
		// Parameter is a preal value: adjacency list is computed by interpolation
		else {
			al.resize(al1.size());
			double ft = std::floor(t), dt = t-ft;
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
				const ds::Graph::adjlist & alt1 = getALt(al1, ft, d, alts),
										 & alt2 = getALt(al1, ft+1, d, alts);
				std::map<long,preal> ne;
				std::map<long,preal>::const_iterator neit;
				for (long i=0, j, n; i<al1.size(); ++i) {
					ne.clear();
					for (j=0; j<alt1[i].size(); ++j)
						ne[alt1[i][j].first] = (1.-dt)*alt1[i][j].second;
					for (j=0; j<alt2[i].size(); ++j) {
						n = alt2[i][j].first;
						if (ne.find(n) == ne.end()) ne[n] = 0.;
						ne[n] += dt*alt2[i][j].second;
					}
					al[i].clear();
					for (neit = ne.begin();	neit != ne.end(); ++neit)
						al[i].push_back(std::make_pair(neit->first,neit->second));
				}
				// Put back memory level if needed
				if (inc_memory) this->memory_saving = 1;
			}
		}
	}
}

const ds::Graph::adjlist & MSCD_SO::getALt(const ds::Graph::adjlist & al1, const int t, const std::vector<preal> & d, std::map<int,ds::Graph::adjlist*> & alts) const {
	assert(t>0);
	// If exists, return
	if (alts.find(t) != alts.end()) return *alts[t];
	if (t==1) return al1;

	// Create list of keys
	std::vector<long> exp;
	std::map<int,ds::Graph::adjlist*>::const_iterator it;
	exp.push_back(1);
	for (it = alts.begin(); it != alts.end(); ++it)
		exp.push_back(it->first);

	// Decompose using larger to smaller exponents of pre-computed networks
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

	// Create list at t and potentially keep intermediate lists for future computations
	int prev = ops.front();
	ops.erase(ops.begin());
	while (!ops.empty()) {
		if (this->verbose)
			fprintf(stdout, "Computing %d from %d and %d\n", prev+ops.front(), prev, ops.front());
		if ((prev > 1) && (alts.find(prev) == alts.end())) { // should not happen
			fprintf(stderr, "Error: Computation requires to keep additional graphs in memory\n-> Increase memory level\n");
			exit(EXIT_FAILURE);
		}
		// Compute new list
		ds::Graph::adjlist * l = new ds::Graph::adjlist;
		alts[prev+ops.front()] = l;
		this->buildNextAL(prev==1?al1:*alts[prev], ops.front()==1?al1:*alts[ops.front()], d, *l);
		// Prepare for next operation
		prev = prev + ops.front();
		ops.erase(ops.begin());
		// Check memory
		this->checkMemory(alts);
	}
	return *alts[t];
}

/// Compute edge values within the given index range
void computeALpart(const long begin, const long end, preal min_edge, const ds::Graph::adjlist & al1, const ds::Graph::adjlist & al2, const std::vector<preal> & d, ds::Graph::adjlist & al) {
	PrintDb("Computing nodes %ld to %ld\n", begin+1, end+1);
	std::map<long,preal> ne;
	std::map<long,preal>::iterator neit;
	preal v, acc;
	for (long i=begin, j, k, n1, n2; i<end; ++i) {
		// Clear edges map
		ne.clear();
		// Compute edges
		for (j=0; j<al1[i].size(); ++j) {
			n1 = al1[i][j].first;
			v = al1[i][j].second/d[i];
			for (k=0; k<al2[n1].size(); ++k) {
				n2 = al2[n1][k].first;
				if (ne.find(n2) == ne.end()) ne[n2] = 0.;
				ne[n2] += v*(al2[n1][k].second/d[n1]);
			}
		}

		// Clear edges
		al[i].clear();
		// Scan for low valued edges and remove them.
		neit = ne.begin();
		v = min_edge/d[i];
		acc = 0.;
		while (neit != ne.end()) {
			if ((neit->first != i) && (neit->second < v)) {
				acc += neit->second;
				ne.erase(neit++);
			}
			else ++neit;
		}
		// Add final edges to the adjacency list
		for (neit = ne.begin();	neit != ne.end(); ++neit)
			al[i].push_back(std::make_pair(neit->first,d[i]*neit->second));
	}
}
    
void MSCD_SO::buildNextAL(const ds::Graph::adjlist & al1, const ds::Graph::adjlist & al2, const std::vector<preal> & d, ds::Graph::adjlist & al) const {
    al.resize(al1.size());
#ifdef USE_MULTITHREADING
    int nb_threads = std::thread::hardware_concurrency();
	if (nb_threads > al1.size()) nb_threads = al1.size();
    PrintDb("Splitting in %d threads\n", nb_threads);
    std::vector<std::thread*> threads(nb_threads);
	long begin, end;
	for (int t=0; t<nb_threads; ++t) {
		begin = static_cast<long>(t*al1.size()/nb_threads);
		end = (t<nb_threads-1)?static_cast<long>((t+1)*al1.size()/nb_threads):al1.size();
		threads[t] = new std::thread(computeALpart, begin, end, this->min_edge, std::cref(al1), std::cref(al2), std::cref(d), std::ref(al));
        PrintDb("Thread %d created with boundaries %ld %ld\n", t+1, begin, end);
	}
	for (int t=0; t<nb_threads; ++t) {
		threads[t]->join();
        PrintDb("Thread %d returns\n", t+1);
		delete threads[t];
	}
#else
    computeALpart(0, al1.size(), this->min_edge, al1, al2, d, al);
#endif
}

#ifdef __DEBUG__
double MSCD_SO::DbCheckQ(const ds::Graph::adjlist & al, const std::vector<long> & ncom, const double i2m) const {
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

/*
 // Keep the best edges only
 std::vector<std::pair<int,preal> > edges;
 for (neit = ne.begin();	neit != ne.end(); ++neit)
 edges.push_back(std::make_pair(neit->first,neit->second));
 std::sort(edges.rbegin(), edges.rend(), comp_edge);
 while (edges.size() > al1[i].size()) edges.pop_back();
 double sum = 0.;
 for (int k=0; k<edges.size(); ++k) sum += edges[k].second;
 for (int k=0; k<edges.size(); ++k) edges[k].second *= d[i]/sum;
 std::sort(edges.begin(), edges.end(), comp_key);
 al[i] = edges;
 //printf("SIZE: %d %d\n", al[i].size(), al1[i].size());
 */

// Transfer the accumulated weight to the self-loop to keep the Markov chain valid
//if (acc > 0.) {
//	if (ne.find(i) == ne.end()) ne[i] = 0.;
//	ne[i] += acc;
//}