/*
 *  mscd_lfk.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 24/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "mscd_lfk.h"
#include "graph.h"
#include "communities.h"
#include <algorithm>
#include <cmath>

// Register algorithm
#include "registry.h"

namespace mscd {
namespace alg {
	
static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_LFK);
/*
long check_duplicates(const std::vector<long> & com) {
	std::vector<long>::const_iterator it = com.begin();
	for (long i=0; i<com.size()-1; ++i)
		if (std::binary_search(++it,com.end(),com[i]))
			return com[i];
	return -1;
}*/
	
bool MSCD_LFK::Run(const ds::Graph & g,
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
	// Weights in and total of communities
	std::vector<double> kin, ktot;
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
	
	// If initial communities are given, initialise relevant variables
	if (!init_coms.IsEmpty()) {
		if (this->verbose) fprintf(stdout, "Starting with %ld given communities", init_coms.GetNbCommunities());
		communities = init_coms;
		std::vector<std::vector<long> > & coms = communities.GetCommunityList();
		std::vector<bool> & has_node = this->bitvector;
		kin.resize(coms.size());
		ktot.resize(coms.size());
		coms_nbs.resize(coms.size());
		for (long i=0, j, k, n; i<coms.size(); ++i) {
			// Flip bits to 1
			for (j=0; j<coms[i].size(); ++j) has_node[coms[i][j]].flip();
			// Initialise given communities
			kin[i] = ktot[i] = 0.;
			for (j=0; j<coms[i].size(); ++j) {
				n = coms[i][j];
				for (k=0; k<al[n].size(); ++k) {
					if (has_node[al[n][k].first]) kin[i] += al[n][k].second;
					else coms_nbs[i].insert(al[n][k].first);
					ktot[i] += al[n][k].second;
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
					//std::advance(it, rand() % candidates.size()); // Enable random access but with linear cost
					n = *it;
					candidates.erase(it);
					// Initialise new community
					std::vector<long> & com = communities.CreateCommunity();
					PrintDb("Starting community %ld from node %ld\n", communities.GetNbCommunities(), n+1);
					com.push_back(n);
					kin.push_back(0.);
					ktot.push_back(0.);
					coms_nbs.resize(coms_nbs.size()+1);
					std::set<long> & nbs = coms_nbs.back();
					for (i=0; i<al[n].size(); ++i) {
						ktot.back() += al[n][i].second;
						nbs.insert(al[n][i].first);
					}
					//dur_i += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - i0);
					// Grow community
					//g0 = clock::now();
					this->grow(al, ps[p], com, kin.back(), ktot.back(), nbs);
					//dur_g += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - g0);
					//c0 = clock::now();
					// Insert in the set of communities to check
					to_check.insert(communities.GetNbCommunities()-1);
					//dur_c += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - c0);
#ifdef __DEBUG__
					double dkin, dktot;
					this->DbCheckKs(al, com, dkin, dktot);
					if ((dkin != kin.back()) || (dktot != ktot.back()))
						fprintf(stderr, "AGI: Incorrect kin/ktot values: kin=%f (%f), ktot=%f (%f)\n", kin.back(), dkin, ktot.back(), dktot);
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
					bool grow_res = this->grow(al, ps[p], coms[i], kin[i], ktot[i], coms_nbs[i]);
					//dur_g += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - g0);
					//m0 = clock::now();
					if (grow_res) {
						merge_com |= true;
						to_check.insert(i);
#ifdef __DEBUG__
						double dkin, dktot;
						this->DbCheckKs(al, communities.GetCommunity(i), dkin, dktot);
						if ((dkin != kin[i]) || (dktot != ktot[i]))
							fprintf(stderr, "AG: Incorrect kin/ktot values: kin=%f (%f), ktot=%f (%f)\n", kin[i], dkin, ktot[i], dktot);
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
								kin[j] = kin.back();				kin.pop_back();
								ktot[j] = ktot.back();				ktot.pop_back();
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
				//done = true;
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
							(((static_cast<double>(n)) >= this->merge_th*coms[c1].size()) ||
							 ((static_cast<double>(n)) >= this->merge_th*coms[c2].size())))
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
							// Update kin and ktot
							kin[c1] += kin[c2];
							ktot[c1] += ktot[c2];
							for (i=0; i<coms[c2].size(); ++i) {
								if (is_ovn_node[coms[c2][i]]) continue;
								x = coms[c2][i];
								for (j=0; j<al[x].size(); ++j)
									if ((c1_has_node[al[x][j].first]) && (!is_ovn_node[al[x][j].first]))
										kin[c1] += 2.*al[x][j].second;
							}
							for (ovnit = ovn_nodes.begin(); ovnit != ovn_nodes.end(); ++ovnit) {
								x = *ovnit;
								for (j=0; j<al[x].size(); ++j) {
									if (is_ovn_node[al[x][j].first])
										kin[c1] -= al[x][j].second;
									ktot[c1] -= al[x][j].second;
								}
							}
							// Update neighbour nodes
							for (i=0; i<coms[c1].size(); ++i) coms_nbs[c2].erase(coms[c1][i]);
							for (i=0; i<coms[c2].size(); ++i) coms_nbs[c1].erase(coms[c2][i]);
							for (it = coms_nbs[c2].begin(); it != coms_nbs[c2].end(); ++it)
								coms_nbs[c1].insert(*it);
							// Put new community as c1
							coms[c1].swap(tmp_com);
							// Update c1 node vector
							for (i=0; i<coms[c2].size(); ++i)
								if (!is_ovn_node[coms[c2][i]]) c1_has_node[coms[c2][i]].flip();
							// Add c1 to the queue of communities to check
							to_check_next.insert(c1);
#ifdef __DEBUG__
							double dkin, dktot;
							this->DbCheckKs(al, coms[c1], dkin, dktot);
							if ((dkin != kin[c1]) || (dktot != ktot[c1]))
								fprintf(stderr, "AM: Incorrect kin/ktot values: kin=%f (%f), ktot=%f (%f)\n", kin[c1], dkin, ktot[c1], dktot);
#endif
							// Delete c2
							coms[c2].swap(coms.back());			coms.pop_back();
							kin[c2] = kin.back();				kin.pop_back();
							ktot[c2] = ktot.back();				ktot.pop_back();
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
		if (kin.size() > 1) {
			for (long i=0; i<kin.size(); ++i) Q += kin[i]/std::pow(ktot[i], ps[p]);
			Q /= kin.size();
		}
		Qs[p] = Q;
		
	} // For each parameter

	//std::cout << "I=" << dur_i.count()/1000. << "  G=" << dur_g.count()/1000. << "  C=" << dur_c.count()/1000. << "  M=" << dur_m.count()/1000. << std::endl;
	
	// Everything went fine
	return true;
}

/// Compare pairs <key,value> for value-based decreasing ordering
struct great_sim {
	bool operator() (const std::pair<long,preal> & p1, const std::pair<long,preal> & p2) const {
		return (p1.second > p2.second) || (p1.first < p2.first);
	}
};

/// Fast growth function with priority queue
bool MSCD_LFK::grow(const ds::Graph::adjlist & al, double a, std::vector<long> & com, double & kin, double & ktot, std::set<long> & nbs) {
	bool changed = false;
	// Set the community membership bits
	std::vector<bool> & has_node = this->bitvector;
	for (long i=0; i<com.size(); ++i) has_node[com[i]].flip();
	// Build priority queue from set
	std::set<std::pair<long,preal>,great_sim> nbs_q;
	std::map<long,preal> nbs_q_rank;
	preal knin, kntot, rank;
	long i;
	for (std::set<long>::iterator it = nbs.begin(); it != nbs.end(); ++it) {
		knin = kntot = 0.;
		for (i=0; i<al[*it].size(); ++i) {
			if (has_node[al[*it][i].first]) knin += al[*it][i].second;
			kntot += al[*it][i].second;
		}
		knin *= 2.;
		rank = knin/std::pow(static_cast<double>(kntot),a);
		nbs_q.insert(std::make_pair(*it,rank));
		nbs_q_rank[*it] = rank;
	}
	// Take all neighbours in order
	long j, n, x;
	preal fg;
	std::set<std::pair<long,preal>,great_sim>::iterator itq;
	std::map<long,preal>::iterator itqr;
	long max_iter = (this->max_nb_failures<=0)?al.size():this->max_nb_failures;
	while (!nbs_q.empty()) {
		// Pick neighbour with highest priority
		n = nbs_q.begin()->first;
		// Remove neighbour
		nbs_q.erase(nbs_q.begin());
		nbs_q_rank.erase(n);
		// Compute kin and ktot
		knin = kntot = 0.;
		for (i=0; i<al[n].size(); ++i) {
			if (has_node[al[n][i].first]) knin += al[n][i].second;
			kntot += al[n][i].second;
		}
		knin *= 2.;
		// Community fitness gain
		fg = (kin+knin)/std::pow(ktot+kntot,a) - kin/std::pow(ktot,a);
		// Allow no improvement no more than k times (considering nodes are sorted)
		if (fg <= 0.) {
			--max_iter;
			if (max_iter <= 0) break;
		}
		else {
			//printf("n=%ld knin=%f kntot=%f kin=%f ktot=%f fg=%f\n", n+1, knin, kntot, kin, ktot, fg);
			// Add node
			PrintDb("Insert %ld (fg = %f) nbs=%ld coms=%ld\n", n+1, fg, nbs.size(), com.size());
			// Add node
			has_node[n].flip();
			com.push_back(n);
			// Update kin, ktot and neighbours
			kin += knin;
			ktot += kntot;
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
					knin = kntot = 0.;
					for (j=0; j<al[x].size(); ++j) {
						if (has_node[al[x][j].first]) knin += al[x][j].second;
						kntot += al[x][j].second;
					}
					knin *= 2.;
					rank = knin/std::pow(static_cast<double>(kntot),a);
					nbs_q.insert(std::make_pair(x,rank));
					nbs_q_rank[x] = rank;
					/*
					// Also update the current queue
					knin = kntot = 0.;
					for (j=0; j<al[x].size(); ++j) {
						if (has_node[al[x][j].first])
							knin += al[x][j].second;
						kntot += al[x][j].second;
					}
					knin *= 2.;
					it = nbs_q.begin();
					while (it != nbs_q.end()) {
						if (it->first == x) break;
						++it;
					}
					if (it != nbs_q.end()) nbs_q.erase(it);
					nbs_q.insert(std::make_pair(x,knin/std::pow(static_cast<double>(kntot),a)));
					 */
				}
			}
			changed = true;
		}
	}
	// If a change has been made to the community, check nodes
	if (changed) {
		// Set the maximum number of iterations allowed
		max_iter = (this->max_check_loops<=0)?com.size():this->max_check_loops;
		long c, k;
		preal gfit, fit;
		bool done;
		do {
			done = true;
			// Recompute community fitness
			gfit = kin/std::pow(ktot,a);
			// Iterate through the nodes
			c=0;
			while (c<com.size()) {
				n  = com[c];
				// Compute fitness without the node
				knin = 0.; kntot = 0.;
				for (i=0; i<al[n].size(); ++i) {
					if (has_node[al[n][i].first]) knin += al[n][i].second;
					kntot += al[n][i].second;
				}
				knin *= 2.;
				// If fitness is better without the node
				fit = (kin - knin) / std::pow(ktot - kntot, a);
				if ((fit > gfit) && this->isStillConnected(al, com, has_node, n)) {
					PrintDb("Remove %ld (f=%f > gf=%f)\n", n+1, fit, gfit);
					// Remove node
					has_node[n].flip();
					com[c] = com.back(); com.pop_back();
					// Update weight in and total
					kin -= knin;
					ktot -= kntot;
					// Remove from the neighbour list some of the node's neighbours
					for (i=0; i<al[n].size(); ++i) {
						j = al[n][i].first;
						if (!has_node[j]) {
							k = 0;
							while (k<al[j].size())
								if (has_node[al[j][k].first]) break;
								else ++k;
							if (k == al[j].size()) nbs.erase(j);
						}
					}
					// Add node to neighbour list
					nbs.insert(n);
					// Recompute community fitness
					gfit = kin/std::pow(ktot,a);
					// Recheck from the beginning
					done = false;
				}
				else ++c;
			}
			// One more iteration achieved
			--max_iter;
		}
		while ((!done) && (max_iter > 0));
		// Update community node list (restore ordered list)
		std::sort(com.begin(), com.end());
	}
	// Unset the community membership bits
	for (i=0; i<com.size(); ++i) has_node[com[i]].flip();
	// Return true if a change occured
	return changed;
}
	
#ifdef __DEBUG__
void MSCD_LFK::DbCheckKs(const ds::Graph::adjlist & al, const std::vector<long> & com, double & kin, double & ktot) const {
	kin = ktot = 0.;
	for (long i=0, j, n; i<com.size(); ++i) {
		n = com[i];
		for (j=0; j<al[n].size(); ++j) {
			if (std::binary_search(com.begin(), com.end(), al[n][j].first))
				kin += al[n][j].second;
			ktot += al[n][j].second;
		}
	}
}
#endif

} // namespace alg
} // namespace mscd
