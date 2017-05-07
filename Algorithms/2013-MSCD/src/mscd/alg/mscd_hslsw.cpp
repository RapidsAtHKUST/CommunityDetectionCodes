/*
 *  mscd_hslsw.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 25/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "mscd_hslsw.h"
#include "graph.h"
#include "communities.h"
#include <algorithm>
#include <cmath>

// Register algorithm
#include "registry.h"

namespace mscd {
namespace alg {

static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_HSLSW);

bool MSCD_HSLSW::Run(const ds::Graph & g,
					 const std::vector<double> & ps,
					 std::vector<ds::Communities> & p_coms,
					 std::vector<double> & Qs,
					 const ds::Communities & init_coms) {
	
	// Get adjacency list
	const ds::Graph::adjlist & al = g.GetAdjList();
	
	// Resize result variables accordingly
	p_coms.resize(ps.size());
	Qs.resize(ps.size());
	
	// Current communities (sorted list of nodes)
	ds::Communities communities;
	// Similarity in and total of communities
	std::vector<double> sin, stot;
	// Neighbour nodes of each community
	std::vector< std::set<long> > coms_nbs;
	// Temporary community buffer and overlapping nodes list
	std::vector<long> tmp_com;
	// Overlapping nodes list
	std::vector<long> ovn_nodes;
	// Set of communities to check for merging, and associated iterator
	std::set<long> to_check, to_check_next;
	std::set<long>::iterator it; 
	
    // Resize community membership bool vector to the appropriate size 
	this->bitvector.resize(al.size());
	this->bitvector.assign(al.size(),false);
	this->bitvector2.resize(al.size());
	this->bitvector2.assign(al.size(),false);
    
	// Similarity between nodes (both symmetrical pairs are stored)
    simmap sims;//(g.GetNbEdges()*2+17);
	//s0 = clock::now();
	this->computeSimilarities(al, sims);
	//dur_s += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - s0);
    
	// If initial communities are given, initialise relevant variables
	if (!init_coms.IsEmpty()) {
		if (this->verbose) fprintf(stdout, "Starting with %ld given communities", init_coms.GetNbCommunities());
		communities = init_coms;
		std::vector<std::vector<long> > & coms = communities.GetCommunityList();
        std::vector<bool> & has_node = this->bitvector;
		sin.resize(coms.size());
		stot.resize(coms.size());
		coms_nbs.resize(coms.size());
		std::pair<long,long> key;
		for (long i=0, j, k; i<coms.size(); ++i) {
			// Flip bits to 1
			for (j=0; j<coms[i].size(); ++j) has_node[coms[i][j]].flip();
            // Initialise given communities
			sin[i] = stot[i] = 0.;
			for (j=0; j<coms[i].size(); ++j) {
				key.first = coms[i][j];
				for (k=0; k<al[key.first].size(); ++k) {
					key.second = al[key.first][k].first;
					if (has_node[key.second]) sin[i] += sims[key];
					else coms_nbs[i].insert(key.second);
					stot[i] += sims[key];
				}
			}
			// Flip bits back to 0
			for (j=0; j<coms[i].size(); ++j) has_node[coms[i][j]].flip();
			// Insert in the set of communities to check
			to_check.insert(i);
		}
	}
    
	// For each parameter (noted gamma in the original paper)
	for (int p=0; p<ps.size(); ++p) {
		
		// Check that the parameter is not negative
		if (ps[p] < 0.) {
			fprintf(stderr, "Error: Parameter value p=%f must be positive\n", ps[p]);
			return false;
		}
		PrintDb("Current p=%f\n", ps[p]);

		// While changes can potentially be made
		bool grow_com = true, merge_com = true;
		while (grow_com) {
			
			// Grow communities
			if (communities.GetNbCommunities() == 0) { // Grow initial communities from seeds
				//i0 = clock::now();
				PrintDb("Grow initial communities\n");
				// Initialise seeds
				std::set<long> candidates;
				std::set<long>::iterator it;
				long i, n;
				std::pair<long,long> key;
				// Create seed candidates set
				for (i=0; i<al.size(); ++i)
					if (al[i].size() > 1) // A seed needs to have at least 2 neighbours
						candidates.insert(i);
				//dur_i += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - i0);
				// While seeds are available
				while (!candidates.empty()) {
					//i0 = clock::now();
					// Pick a node
					it = candidates.begin();
					//std::advance(it, rand() % seeds.size()); // Enable random access but with linear cost
					n = *it;
					candidates.erase(it);
					// Initialise new community
					std::vector<long> & com = communities.CreateCommunity();
					PrintDb("Starting community %ld from node %ld\n", communities.GetNbCommunities(), n+1);
					com.push_back(n);
					sin.push_back(0.);
					stot.push_back(0.);
					coms_nbs.resize(coms_nbs.size()+1);
					std::set<long> & nbs = coms_nbs.back();
					for (i=0; i<al[n].size(); ++i) {
						stot.back() += sims[std::make_pair(n,al[n][i].first)];
						nbs.insert(al[n][i].first);
					}
					//dur_i += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - i0);
					// Grow community
					//g0 = clock::now();
					this->grow(al, sims, ps[p], com, sin.back(), stot.back(), nbs);
					//dur_g += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - g0);
					//c0 = clock::now();
					// Insert in the set of communities to check
					to_check.insert(communities.GetNbCommunities()-1);
					//dur_c += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - c0);
#ifdef __DEBUG__
					double dsin, dstot;
					this->DbCheckSims(al, sims, com, dsin, dstot);
					if ((std::fabs(dsin-sin.back()) > ERROR_Q_DIFF) || (std::fabs(dstot-stot.back()) > ERROR_Q_DIFF))
						fprintf(stderr, "AGI: Incorrect sin/stot values: sin=%f (%f), stot=%f (%f)\n", sin.back(), dsin, stot.back(), dstot);
#endif
					//i0 = clock::now();
					// Remove from the seed candidates the nodes found in the community
					for (i=0; i<com.size(); ++i) candidates.erase(com[i]);
					//dur_i += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - i0);
				}
				if (this->verbose) fprintf(stdout, "%ld initial communities created", communities.GetNbCommunities());
			}
			else {	// Grow existing communities
				PrintDb("Grow existing communities (%ld)\n", communities.GetNbCommunities());
				std::vector<std::vector<long> > & coms = communities.GetCommunityList();
				for (long i=0, j, s, ds; i<coms.size(); ++i) {
					PrintDb("Growing community %ld\n", i+1);
                    s = coms[i].size();
					//g0 = clock::now();
					bool grow_res = this->grow(al, sims, ps[p], coms[i], sin[i], stot[i], coms_nbs[i]);
					//dur_g += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - g0);
					//m0 = clock::now();
					if (grow_res) {
						merge_com |= true;
						to_check.insert(i);
#ifdef __DEBUG__
						double dsin, dstot;
						this->DbCheckSims(al, sims, communities.GetCommunity(i), dsin, dstot);
						if ((std::fabs(dsin-sin[i]) > ERROR_Q_DIFF) || (std::fabs(dstot-stot[i]) > ERROR_Q_DIFF))
						fprintf(stderr, "AG: Incorrect sin/stot values: sin=%f (%f), stot=%f (%f)\n", sin[i], dsin, stot[i], dstot);
#endif
						if (coms[i].size() <= s) continue;
						// Compute size increase
						ds = coms[i].size() - s;
						// Remove communities not grown yet if they are encompassed within the latest grown community
						j = i+1;
						while (j<coms.size()) {
							// if ci is bigger than cj, grew enough to fully emcompass cj knowing they did not merge, and actually encompasses cj 
							if ((coms[i].size()>=coms[j].size()) &&
								(static_cast<double>(ds)>coms[j].size()-this->merge_th*(s<coms[j].size()?s:static_cast<long>(coms[j].size()))) &&
								this->encompass(coms[i], coms[j])) {
								coms[j].swap(coms.back());			coms.pop_back();
								sin[j] = sin.back();				sin.pop_back();
								stot[j] = stot.back();				stot.pop_back();
								coms_nbs[j].swap(coms_nbs.back());	coms_nbs.pop_back();	
							}
							else ++j;
						}
					}
					//dur_m += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - m0);
				}
			}
			// Communities need not be grown again unless communites are merged
			grow_com = false;

			// If no need to merge communities, jump the merging section
			if (!merge_com) break;
            
			//m0 = clock::now();
			// Merge communities if they share a significant amount of nodes
			do {
				PrintDb("Merge communities (th=%f) (%ld coms)\n", this->merge_th, communities.GetNbCommunities());
				// For each community pair
				std::vector<std::vector<long> > & coms = communities.GetCommunityList();
				long i, j, k, n, c1, c2, x;
                std::vector<long>::iterator ovnit;
                std::vector<bool>   & c1_has_node = this->bitvector,
                                    & is_ovn_node = this->bitvector2;
				// While there are communities to check
				while (!to_check.empty()) {
					// Take the first one
					c1 = *to_check.begin();
					// Remove it from the set
					to_check.erase(to_check.begin());
					// Set bits for c1
					for (i=0; i<coms[c1].size(); ++i) c1_has_node[coms[c1][i]].flip();
                    // Compare with c2
					c2 = 0;
					while (c2 < coms.size()) {
						// Indices must be different
						if (c1 == c2) { ++c2; continue; }
						// Compute the number of shared nodes
						n = this->nb_shared_nodes(coms[c1],coms[c2]);
						// If they share more than a given ratio, merge
						if ((n > 0) &&
							(((static_cast<double>(n))/coms[c1].size() >= this->merge_th) ||
							 ((static_cast<double>(n))/coms[c2].size() >= this->merge_th)))
						{
							PrintDb("Merging %ld (%f) and %ld (%f)\n", c1+1, (static_cast<double>(n))/coms[c1].size(), c2+1, (static_cast<double>(n))/coms[c2].size());
							// Buffer for merged community
							tmp_com.resize(coms[c1].size()+coms[c2].size()-n);
							// Merge and process each new node from c2 added to c1
							ovn_nodes.resize(n);
							i = j = k = 0, x = 0;
							while ((i < coms[c1].size()) && (j < coms[c2].size())) {
								if (coms[c1][i] <= coms[c2][j]) {
									tmp_com[k++] = coms[c1][i];
									if (coms[c1][i] == coms[c2][j]) {
										is_ovn_node[coms[c1][i]].flip();
										ovn_nodes[x++] = coms[c1][i];
										++j;
									}
									++i;
								}
								else tmp_com[k++] = coms[c2][j++];
							}
							while (i < coms[c1].size()) tmp_com[k++] = coms[c1][i++];
							while (j < coms[c2].size()) tmp_com[k++] = coms[c2][j++];
							// Update sin and stot
							sin[c1] += sin[c2];
							stot[c1] += stot[c2];
							for (i=0; i<coms[c2].size(); ++i) {
								x = coms[c2][i];
								if (is_ovn_node[coms[c2][i]]) continue;
								for (j=0; j<al[x].size(); ++j)
									if ((c1_has_node[al[x][j].first]) && (!is_ovn_node[al[x][j].first]))
										sin[c1] += 2.*sims[std::make_pair(x,al[x][j].first)];
							}
							for (ovnit = ovn_nodes.begin(); ovnit != ovn_nodes.end(); ++ovnit) {
								x = *ovnit;
								for (j=0; j<al[x].size(); ++j) {
									if (is_ovn_node[al[x][j].first])
										sin[c1] -= sims[std::make_pair(x,al[x][j].first)];
									stot[c1] -= sims[std::make_pair(x,al[x][j].first)];
								}
							}
							// Update neighbour nodes
							for (i=0; i<coms[c1].size(); ++i) coms_nbs[c2].erase(coms[c1][i]);
							for (i=0; i<coms[c2].size(); ++i) coms_nbs[c1].erase(coms[c2][i]);
							for (it = coms_nbs[c2].begin(); it != coms_nbs[c2].end(); ++it)
								coms_nbs[c1].insert(*it);
							// Put new community instead of l, swap s with last, delete last
							coms[c1].swap(tmp_com);
							// Update c1 node vector
							for (i=0; i<coms[c2].size(); ++i)
								if (!is_ovn_node[coms[c2][i]]) c1_has_node[coms[c2][i]].flip();
							// Add c1 to the queue of communities to check
							to_check_next.insert(c1);
#ifdef __DEBUG__
							double dsin, dstot;
							this->DbCheckSims(al, sims, coms[c1], dsin, dstot);
							if ((std::fabs(dsin-sin[c1]) > ERROR_Q_DIFF) || (std::fabs(dstot-stot[c1]) > ERROR_Q_DIFF))
								fprintf(stderr, "AM: Incorrect sin/stot values: sin=%f (%f), stot=%f (%f)\n", sin[c1], dsin, stot[c1], dstot);
#endif
							// Delete c2
							coms[c2].swap(coms.back());			coms.pop_back();
							sin[c2] = sin.back();				sin.pop_back();
							stot[c2] = stot.back();				stot.pop_back();
							coms_nbs[c2].swap(coms_nbs.back());	coms_nbs.pop_back();
							// Remove c2 from communities to check
							to_check.erase(c2);
							to_check_next.erase(c2);
							// Rename last community to c2 if found in communities to check
							it = to_check.find(coms.size());
							if (it != to_check.end()) {
								to_check.erase(it);
								to_check.insert(c2);
							}
							it = to_check_next.find(coms.size());
							if (it != to_check_next.end()) {
								to_check_next.erase(it);
								to_check_next.insert(c2);
							}
							// If this community was c1, update c1 index
							if (c1 == coms.size()) c1 = c2;
							// Unset overlapping nodes
							for (ovnit = ovn_nodes.begin(); ovnit != ovn_nodes.end(); ++ovnit) is_ovn_node[*ovnit].flip();
							// More computation may be required
							grow_com = true;
							// Exit from current loop
							c2 = coms.size();
						}
						else ++c2;
					}
					// Unset bits
					for (i=0; i<coms[c1].size(); ++i) c1_has_node[coms[c1][i]].flip();
				}
				// Set the new list of communities to check
				// (delaying to the next loop the checking of the newly merged communities may prevent the
				// early formation of a large community encompassing all its small neighbours successively)
				to_check.swap(to_check_next);
			}
			while(!to_check.empty()); // While communities can potentially be merged
			// Communities need not be checked again unless nodes are moved
			merge_com = false;
			//dur_m += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - m0);

		} // While changes can potentially be made
		
		// Store final partition for the current parameter
		this->storeCommunities(communities, p_coms[p]);
		double Q = 0.;
		if (sin.size() > 1) {
			for (long i=0; i<sin.size(); ++i) Q += sin[i]/stot[i];
			Q /= sin.size();
		}
		Qs[p] = Q;
		
	} // For each parameter
	
	//std::cout << "S=" << dur_s.count()/1000. << "  I=" << dur_i.count()/1000. << "  G=" << dur_g.count()/1000. << "  C=" << dur_c.count()/1000. << "  M=" << dur_m.count()/1000. << std::endl;
	
	// Everything went fine
	return true;
}

/// Compare pairs <key,value> for value-based decreasing ordering
struct great_sim {
	bool operator() (const std::pair<long,preal> & p1, const std::pair<long,preal> & p2) const {
		return (p1.second > p2.second) || (p1.first < p2.first);
	}
};

/// Compare pairs <key,value>) for key-based increasing ordering
bool comp_key(const std::pair<long,preal> & p1, const std::pair<long,preal> & p2) {
	return p1.first < p2.first;
}

bool MSCD_HSLSW::grow(const ds::Graph::adjlist & al, const simmap & sims, double a, std::vector<long> & com, double & sin, double & stot, std::set<long> & nbs) {
	bool changed = false;
    // Set the community membership bits
	std::vector<bool> & has_node = this->bitvector;
	for (long i=0; i<com.size(); ++i) has_node[com[i]].flip();
	// Build priority queue from set
	std::set<std::pair<long,preal>,great_sim> nbs_q;
	std::pair<long,long> key;
	std::map<long,preal> nbs_q_rank;
	preal max_s;
	long i;
	for (std::set<long>::iterator it = nbs.begin(); it != nbs.end(); ++it) {
		max_s = 0.;
		key.first = *it;
		for (i=0; i<al[*it].size(); ++i) {
			if (has_node[al[*it][i].first]) {
				key.second = al[*it][i].first;
				if (sims.at(key) > max_s) max_s = sims.at(key);
			}
		}
		nbs_q.insert(std::make_pair(*it,max_s));
		nbs_q_rank[*it] = max_s;
	}
	// Take all neighbours in order
    long j, n, x;
	preal snin, sntot, s, tn;
    std::set<std::pair<long,preal>,great_sim>::iterator itq;
	std::map<long,preal>::iterator itqr;
	while (!nbs_q.empty()) {
		// Pick neighbour with highest similarity
		n = nbs_q.begin()->first;
		// Remove neighbour
		nbs_q.erase(nbs_q.begin());
		nbs_q_rank.erase(n);
		// Compute tighness gain
		snin = sntot = 0.;
		key.first = n;
		for (i=0; i<al[n].size(); ++i) {
			key.second = al[n][i].first;
			s = sims.at(key);
			if (has_node[al[n][i].first])
				snin += s;
			sntot += s;
		}
		if (sin > 0.) tn = (stot - sin)/sin - (a*(sntot-snin)-snin)/(2.*snin);
		else tn = 1.;
		// If gain is positive
		if (tn > 0.) {
			PrintDb("Insert %ld (Tn=%f > 0)\n", n+1, tn);
            // Add node
			has_node[n].flip();
			com.push_back(n);
			// Update sin, stot and nbs
			sin += 2.*snin;
			stot += sntot;
			nbs.erase(n);
			for (i=0; i<al[n].size(); ++i) {
				if (!has_node[al[n][i].first]) {
					x = al[n][i].first;
					// Add it to the neighbour list
					nbs.insert(x);
					// Update the neighbour queue
					itqr = nbs_q_rank.find(x);
					if (itqr != nbs_q_rank.end()) {
						//auto itq = nbs_q.find(std::make_pair(x,itqr->second));
						itq = std::find(nbs_q.begin(), nbs_q.end(),std::make_pair(x,itqr->second));
						if (itq != nbs_q.end()) nbs_q.erase(itq);
					}
					max_s = 0.;
					key.first = x;
					for (j=0; j<al[x].size(); ++j) {
						if (has_node[al[x][j].first]) {
							key.second = al[x][j].first;
							if (sims.at(key) > max_s) max_s = sims.at(key);
						}
					}
					nbs_q.insert(std::make_pair(x,max_s));
					nbs_q_rank[x] = max_s;
					/*
					// Also update the current queue
					max_s = 0.;
					key.first = x;
					for (j=0; j<al[x].size(); ++j) {
						if (has_node[al[x][j].first]) {
							key.second = al[x][j].first;
							if (sims.at(key) > max_s) max_s = sims.at(key);
						}
					}
					it = nbs_q.begin();
					while (it != nbs_q.end()) {
						if (it->first == x) break;
						++it;
					}
					if (it != nbs_q.end()) nbs_q.erase(it);
					nbs_q.insert(std::make_pair(x,max_s));
					 */
				}
			}
			changed = true;
		}
	}
    // Update community node list if necessary (restore ordered list)
    if (changed) std::sort(com.begin(), com.end());
	// Unset the community membership bits
	for (i=0; i<com.size(); ++i) has_node[com[i]].flip();
    // Return true if a change occured
	return changed;
}

void MSCD_HSLSW::computeSimilarities(const ds::Graph::adjlist & al, simmap & sims) const {
	// Compute sum of squared weights for each node
	std::vector<preal> wxk2(al.size());
	for (long n=0, i; n<al.size(); ++n) {
		wxk2[n] = 0.;
		for (i=0; i<al[n].size(); ++i)
			wxk2[n] += al[n][i].second * al[n][i].second;
	}
	// Compute product of 2 edges weight linking two nodes through a third node
	std::vector< std::pair<long,preal> >::const_iterator it;
	std::pair<long,preal> key1, key2;
	preal wxky;
	for (long n1=0, n2, i, j; n1<al.size(); ++n1) {
		key1.first = key2.second = n1;
		for (i=0; i<al[n1].size(); ++i) {
			if (al[n1][i].first < i) continue;
			key1.second = key2.first = n2 = al[n1][i].first;
			// Compute similarities using common neighbours
			wxky = 0.;
			for (j=0; j<al[n1].size(); ++j) {
				it = std::lower_bound(al[n2].begin(), al[n2].end(), std::make_pair(al[n1][j].first,0.), comp_key);
				if ((it != al[n2].end()) && (it->first == al[n1][j].first))
					wxky += al[n1][j].second * it->second;
			}
			// Add the direct link contribution
			wxky += 2.*al[n1][i].second;
			// Create entry
			sims[key1] = sims[key2] = wxky / (sqrt(wxk2[n1]) * sqrt(wxk2[n2]));
		}
	}
	PrintDb("Similarity map contains %ld entries\n", static_cast<long>(sims.size()));
/*
	for (long i=0; i<al.size(); ++i) {
		printf("%d: ", i+1);
		for (long j=0; j<al[i].size(); ++j)
			printf("(%d,%f) ", al[i][j].first+1, sims[std::make_pair(i, al[i][j].first)]);
		printf("\n");
	}
*/	
}
	
#ifdef __DEBUG__
void MSCD_HSLSW::DbCheckSims(const ds::Graph::adjlist & al, const simmap & sims, const std::vector<long> & com, double & sin, double & stot) const {
	sin = stot = 0.;
	simmap::const_iterator it;
	for (long i=0, j, n; i<com.size(); ++i) {
		n = com[i];
		for (j=0; j<al[n].size(); ++j) {
			it = sims.find(std::make_pair(n,al[n][j].first));
			if (it == sims.end()) {
				fprintf(stderr, "Error: No similarity value for node pair (%ld,%ld)\n", n+1, al[n][j].first+1);
				continue;
			}
			if (std::binary_search(com.begin(), com.end(), al[n][j].first))
				sin += it->second;
			stot += it->second;
		}
	}
}
#endif
	
} // namespace alg
} // namespace mscd
