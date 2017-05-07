//
//  mscd_lfk2.cpp
//  MSCD
//
//  Created by Erwan Le Martelot on 18/04/2012.
//  Copyright (c) 2012 Erwan Le Martelot. All rights reserved.
//

#include "mscd_lfk2.h"
#ifdef USE_MULTITHREADING

#include "graph.h"
#include "communities.h"
#include <thread>

// Register algorithm
#include "registry.h"

// Redefine Debug printing macro (only prints in debug mode) for multi-threading
#ifdef __DEBUG__
#include <mutex>
#undef PrintDb
static std::mutex print_mut;
#define PrintDb(...) {print_mut.lock(); fprintf(stdout, __VA_ARGS__); print_mut.unlock();}
#endif

namespace mscd {
namespace alg {
	
static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_LFK2);

/// Grow communities thread callback
void thread_grow_func(MSCD_LFK2 & inst, MSCD_LFK2::ThreadGrowData & data) {
	for (long i=data.begin, c; i<data.end; ++i) {
		c = data.coms_ids[i];
		data.ret_value[i-data.begin] = inst.grow(data.al, data.a, c, data.ncoms, data.coms[c], data.kin[c], data.ktot[c], data.nbs[c], data.bv1, data.bv2);
	}
}

/// Check communities to merge thread callback
void thread_check_func(MSCD_LFK2 & inst, MSCD_LFK2::ThreadCheckData & data) {
	auto it = data.check_coms.begin(); std::advance(it, data.begin);
	for (long i=0; i<data.nb; ++i, ++it)
		inst.check(data.al, *it, data.coms[*it], data.ncoms, data.to_merge);
}
	
/// Merge communities thread callback
void thread_merge_func(MSCD_LFK2 & inst, MSCD_LFK2::ThreadMergeData & data) {
	long c1, c2;
	for (auto it=data.thr_ops.begin(); it != data.thr_ops.end(); ++it) {
		c1 = it->first;
		c2 = it->second;
		inst.merge(data.al, c1, c2, data.coms[c1], data.nbs[c1], data.kin[c1], data.ktot[c1], data.coms[c2], data.nbs[c2], data.kin[c2], data.ktot[c2], data.bv1, data.bv2);
	}
}
	
bool MSCD_LFK2::Run(const ds::Graph & g,
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
	// Reference to the community list data
	std::vector<std::vector<long> > & coms = communities.GetCommunityList();
	// Community memberships for each node
	std::vector< toolkit::atomic_set<long> > ncoms(al.size());
	// Weights in and total of communities
	std::vector<double> kin, ktot;
	// Neighbour nodes of each community
	std::vector< std::set<long> > coms_nbs;
	
	// If initial communities are given, initialise relevant variables
	//i0 = clock::now();
	if (!init_coms.IsEmpty()) {
		if (this->verbose) fprintf(stdout, "Starting with %ld given communities", init_coms.GetNbCommunities());
		communities = init_coms;
		kin.resize(coms.size());
		ktot.resize(coms.size());
		coms_nbs.resize(coms.size());
		for (long i=0, j, k, n; i<coms.size(); ++i) {
			// Initialise given communities
			kin[i] = ktot[i] = 0.;
			for (j=0; j<coms[i].size(); ++j) {
				n = coms[i][j];
				for (k=0; k<al[n].size(); ++k) {
					if (ncoms[al[n][k].first].find(i))
						kin[i] += al[n][k].second;
					else coms_nbs[i].insert(al[n][k].first);
					ktot[i] += al[n][k].second;
				}
			}
		}
	}
	// If no community exists, determine starting points
	else {
		// Set the maximum number of seeds
		long max_seeds = (this->max_nb_seeds==0)?al.size():this->max_nb_seeds;
		PrintDb("Create initial communities\n");
		// Select random seeds not neighbours of previously selected seeds
		std::set<long> candidates, seeds;
		for (long i=0; i<al.size(); ++i)
			if (al[i].size() > 1) // A seed needs to have at least 2 neighbours
				candidates.insert(i);
		// Chose all the initial seeds
		while ((!candidates.empty()) && (seeds.size() < max_seeds)) {
			// Pick a seed from the candidate list and insert it in the selected seeds list
			auto seed_it = candidates.begin();
			//std::advance(seed_it, rand() % candidates.size()); // Enable random access but with linear cost
			seeds.insert(*seed_it);
			if (this->seed_ngh_level > 0) {
				// Remove its neighbours from the candidate list
				for (auto it=al[*seed_it].begin(); it != al[*seed_it].end(); ++it) {
					if (it->first != *seed_it) candidates.erase(it->first);
					if (this->seed_ngh_level > 1) {
						// Remove the neighbours of its neighbours
						for (auto it2=al[it->first].begin(); it2 != al[it->first].end(); ++it2)
							if (it2->first != *seed_it) candidates.erase(it2->first);
					}
				}
			}
			// The seed is no longer candidate either
			candidates.erase(seed_it);
		}
		// Initialise structures
		kin.resize(seeds.size());
		ktot.resize(seeds.size());
		coms_nbs.resize(seeds.size());
		long c = 0;
		for (auto it = seeds.begin(); it != seeds.end(); ++it, ++c) {
			communities.CreateCommunity().push_back(*it);
			kin[c] = 0;
			ktot[c] = 0.;
			for (long i=0; i<al[*it].size(); ++i) {
				ktot[c] += al[*it][i].second;
				coms_nbs[c].insert(al[*it][i].first);
			}
		}
	}
	//dur_i += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - i0);
	//printf("nbseeds=%ld\n", communities.GetNbCommunities());

	// Threads data
	long nb_threads = (this->max_nb_threads==0)?std::thread::hardware_concurrency():this->max_nb_threads;
	std::vector<std::thread> threads(nb_threads);
	std::vector<ThreadGrowData*> thread_grow_data(nb_threads);
	std::vector<ThreadCheckData*> thread_check_data(nb_threads);
	std::vector<ThreadMergeData*> thread_merge_data(nb_threads);
	std::vector< std::pair< std::vector<bool>, std::vector<bool> > > bitvectors(nb_threads);
	for (long b=0; b<bitvectors.size(); ++b) {
		bitvectors[b].first.resize(al.size(), false);
		bitvectors[b].second.resize(al.size(), false);
	}
	
	// To grow flag: indicates whether a community should be grown
	std::vector<bool> to_grow(coms.size());
	// List of communities to grow, built based on the flag value
	std::vector<long> coms_ids(coms.size());
	
	// For each parameter (noted gamma in the original paper)
	for (int p=0; p<ps.size(); ++p) {
		
		// Check that the parameter is not negative
		if (ps[p] < 0.) {
			fprintf(stderr, "Error: Parameter value p=%f must be positive\n", ps[p]);
			return false;
		}
		PrintDb("Current p=%f\n", ps[p]);
	
		// For each new scale, all communities can be grown
		to_grow.assign(to_grow.size(), true);
		coms_ids.resize(to_grow.size());
		
		bool changed = (coms.size() > 1) ? true : false;
		while (changed) {
			changed = false;

			// Reset memberships
			//r0 = clock::now();
			for (auto it=ncoms.begin(); it != ncoms.end(); ++it) it->clear();
			for (long i=0; i<coms.size(); ++i)
				for (auto it=coms[i].begin(); it != coms[i].end(); ++it)
					ncoms[*it].insert_na(i);
			//dur_r += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - r0);
			
			// Grow communities independently
			//g0 = clock::now();
			PrintDb("Grow communities (%ld)\n", communities.GetNbCommunities());
			// Get the list of communities to grow
			long cid_size = 0;
			for (long i=0; i<to_grow.size(); ++i)
				if (to_grow[i]) coms_ids[cid_size++] = i;
			// Number of threads necessary to grow communities
			long nb_thr = nb_threads;
			if (nb_thr > cid_size) nb_thr = cid_size;
			// Grow communities threads
			for (long t=0, begin, end; t<nb_thr; ++t) {
				// Get communities to process
				begin = static_cast<long>(t*cid_size/nb_thr);
				end = (t<nb_thr-1)?static_cast<long>((t+1)*cid_size/nb_thr):cid_size;
				// Launch growth thread
				thread_grow_data[t] = new ThreadGrowData(al, ps[p], coms_ids, begin, end, ncoms, coms, kin, ktot, coms_nbs, bitvectors[t].first, bitvectors[t].second);
				threads[t] = std::thread(thread_grow_func, std::ref(*this), std::ref(*thread_grow_data[t]));
				PrintDb("Grow thread %ld created with boundaries %ld %ld\n", t+1, begin, end);
			}
			std::set<long> to_check;
			for (long t=0, i, c; t<nb_thr; ++t) {
				// Terminate threads
				threads[t].join();
				// Collect data
				for (i=thread_grow_data[t]->begin; i<thread_grow_data[t]->end; ++i) {
					c = coms_ids[i];
					if (thread_grow_data[t]->ret_value[i-thread_grow_data[t]->begin]) {
						to_check.insert(c);
						changed = true;
					}
					to_grow[c] = false;
				}
				delete thread_grow_data[t];
			}
			//dur_g += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - g0);
						
			// If some communities have been modified
			if (!to_check.empty()) {
				//c0 = clock::now();
				// Checks communities to find the ones to merge
				std::set< std::pair<long,long> > to_merge;
				nb_thr = nb_threads;
				if (nb_thr > to_check.size()) nb_thr = to_check.size();
				// Check communities threads
				for (long t=0, begin, nb; t<nb_thr; ++t) {
					// Get the communities to process
					begin = static_cast<long>(t*to_check.size()/nb_thr);
					nb = (t<nb_thr-1)?static_cast<long>(to_check.size()/nb_thr):(to_check.size()-begin);
					// Launch check thread
					thread_check_data[t] = new ThreadCheckData(al, to_check, begin, nb, coms, ncoms);
					threads[t] = std::thread(thread_check_func, std::ref(*this), std::ref(*thread_check_data[t]));
					PrintDb("Check thread %ld created with first index %ld and range %ld\n", t+1, begin, nb);
				}
				for (long t=0; t<nb_thr; ++t) {
					// Terminate threads
					threads[t].join();
					// Collect data
					to_merge.insert(thread_check_data[t]->to_merge.begin(), thread_check_data[t]->to_merge.end());
					delete thread_check_data[t];
				}
				//dur_c += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - c0);

				// If there are communities to merge
				if (!to_merge.empty()) {
					//m0 = clock::now();
					// Merge the required communities
					nb_thr = nb_threads;
					if (nb_thr > to_merge.size()) nb_thr = to_merge.size();
					std::set<long> thr_coms;
					std::vector< std::set< std::pair<long,long> > > thr_ops(nb_thr);
					std::map<long,long> com_new_idx;
					long c1, c2, tc, t, first_t;
					bool found;
					// While more communities have to be merged
					while (!to_merge.empty()) {
						thr_coms.clear();
						for (auto it=thr_ops.begin(); it != thr_ops.end(); ++it) it->clear();
						first_t = 0;
						com_new_idx.clear();
						// Iterate through pairs of communities to merge
						auto it = to_merge.begin();
						while (it != to_merge.end()) {
							c1 = it->first;
							c2 = it->second;
							found = false;
							for (tc=0; (tc<nb_thr) && (!found); ++tc) {
								t = (first_t + tc) % nb_thr;
								if ((thr_coms.find(c1) == thr_coms.end()) && (thr_coms.find(c2) == thr_coms.end())) {
									thr_ops[t].insert(*it);
									thr_coms.insert(c1);
									thr_coms.insert(c2);
									com_new_idx[c2] = c1;
									++first_t; if (first_t >= nb_thr) first_t = 0;
									to_merge.erase(it++);
									found = true;
								}
							}
							if (!found) ++it;
							// Community c1 will need to be check for growth
							to_grow[c1] = true;
						}
						// Merge communities threads
						for (long t=0; t<nb_thr; ++t) {
							// Launch merge thread
							thread_merge_data[t] = new ThreadMergeData(al, thr_ops[t], coms, kin, ktot, coms_nbs, bitvectors[t].first, bitvectors[t].second);
							threads[t] = std::thread(thread_merge_func, std::ref(*this), std::ref(*thread_merge_data[t]));
							PrintDb("Merge thread %ld created with %ld ops\n", t+1, thr_ops[t].size());
						}
						for (long t=0; t<nb_thr; ++t) {
							// Terminate threads
							threads[t].join();
							delete thread_merge_data[t];
						}
						// Replace all references to c2 communities left with references to the community c1 they joined
						std::set< std::pair<long,long> > new_to_merge;
						std::map<long,long>::iterator mit;
						for (it = to_merge.begin(); it != to_merge.end(); ++it) {
							mit = com_new_idx.find(it->first);
							c1 = (mit != com_new_idx.end()) ? mit->second : it->first;
							mit = com_new_idx.find(it->second);
							c2 = (mit != com_new_idx.end()) ? mit->second : it->second;
							new_to_merge.insert((c1<c2)?std::make_pair(c1,c2):std::make_pair(c2,c1));
						}
						to_merge.swap(new_to_merge);
					}
					//dur_m += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - m0);
				}

				// Remove empty communities
				//e0 = clock::now();
				long t=coms.size()-1;
				while (t >= 0) {
					if (coms[t].empty()) {
						if (t < coms.size()-1) {
							coms[t].swap(coms.back());
							kin[t] = kin.back();
							ktot[t] = ktot.back();
							coms_nbs[t].swap(coms_nbs.back());
							to_grow[t] = to_grow.back();
						}
						coms.pop_back();
						kin.pop_back();
						ktot.pop_back();
						coms_nbs.pop_back();
						to_grow.pop_back();
					}
					--t;
				}
				//dur_e += std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - e0);
				
			}
						
		}
	
		// Store final partition for the current parameter
		this->storeCommunities(communities, p_coms[p]);
		double Q = 0.;
		if (kin.size() > 1) {
			for (long i=0; i<kin.size(); ++i) Q += kin[i]/std::pow(ktot[i], ps[p]);
			Q /= kin.size();
		}
		Qs[p] = Q;
		
	} // For each parameter

	//std::cout << "I=" << dur_i.count()/1000. << "  R=" << dur_r.count()/1000. << "  G=" << dur_g.count()/1000. << "  C=" << dur_c.count()/1000. << "  M=" << dur_m.count()/1000. << "  E=" << dur_e.count()/1000. << std::endl;
	
	// Everything went fine
	return true;
}

/// Compare pairs <key,value> for value-based decreasing ordering
struct great_sim {
	bool operator() (const std::pair<long,preal> const & p1, const std::pair<long,preal> const & p2) const {
		return (p1.second > p2.second) || (p1.first < p2.first);
	}
};

/// Fast growth function with priority queue (called in parallel by threads)
bool MSCD_LFK2::grow(const ds::Graph::adjlist & al, // adjacency list
					 const double a, // alpha parameter
					 const long cid, // community id
					 std::vector< toolkit::atomic_set<long> > & ncoms, // community memberships for each node (atomic)
					 std::vector<long> & com, // community nodes
					 double & kin, // community inside edges
					 double & ktot, // community total edges
					 std::set<long> & nbs, // community neighbours
					 std::vector<bool> & allocated_bitvector1,
					 std::vector<bool> & allocated_bitvector2
					) {
	bool changed = false;
	// Count the number of overlapping nodes per community
	std::map<long,long> ovn_nodes;
	std::vector<long> other_coms;
	for (auto nit = com.begin(); nit != com.end(); ++nit) {
		ncoms[*nit].items(other_coms);
		for (auto it = other_coms.begin(); it != other_coms.end(); ++it) {
			if (*it != cid) {
				++ovn_nodes[*it];
				// If enough overlapping nodes the community should be merged in to the larger one
				if (ovn_nodes[*it] >= this->merge_th * com.size()) return true;
			}
		}
	}
	//printf("A\n");
	// Set community as a bit vector for speed and avoid concurrent reading access to ncoms
	std::vector<bool> & has_node = allocated_bitvector1;
	for (auto nit = com.begin(); nit != com.end(); ++nit) has_node[*nit].flip();
	// Build priority queue from set
	std::set<std::pair<long,preal>,great_sim> nbs_q;
	std::map<long,preal> nbs_q_rank;
	preal knin, kntot, rank;
	long i;
	for (auto it = nbs.begin(); it != nbs.end(); ++it) {
		knin = kntot = 0.;
		for (i=0; i<al[*it].size(); ++i) {
			if (has_node[al[*it][i].first])
				knin += al[*it][i].second;
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
	while (!nbs_q.empty()) {
		// Pick neighbour with highest priority
		n = nbs_q.begin()->first;
		/*
		if (nbs_q_rank.find(n) == nbs_q_rank.end()) {
			printf("Cannot find and delete %ld (with %f)\n", n, nbs_q.begin()->second);
			exit(0);
		}
		else {
			printf("Found %ld %f\n", n, nbs_q_rank.find(n)->second);
		}*/
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
		// If positive improvement, add node
		if (fg > 0.) {
			//printf("n=%ld knin=%f kntot=%f kin=%f ktot=%f fg=%f\n", n+1, knin, kntot, kin, ktot, fg);
			// Add node
			PrintDb("Insert %ld - size(nbs)=%ld - size(com)=%ld\n", n+1, nbs.size(), com.size());
			// Add node
			com.push_back(n);
			// Update community list
			ncoms[n].insert(cid);
			has_node[n].flip();
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
					//printf("Looking for %ld\n", x);
					auto itqr = nbs_q_rank.find(x);
					if (itqr != nbs_q_rank.end()) {
						//printf("Found in rank map with %f\n", itqr->second);
						//auto itq = nbs_q.find(std::make_pair(x,itqr->second));
						auto itq = std::find(nbs_q.begin(), nbs_q.end(),std::make_pair(x,itqr->second));
						/*
						if (itq != itq2) {
							printf("Problem in find:\n");
							for (auto itx = nbs_q.begin(); itx != nbs_q.end(); ++itx) {
								printf("(%ld,%f) ", itx->first, itx->second);
								if ((itx->first == x) && (itx->second == itqr->second)) printf("X ");
							}
							printf("\n");
							exit(1);
						}
						 */
						if (itq != nbs_q.end()) {
							//printf("Erase: %ld %f (looking for %ld %f) - %ld\n", itq->first, itq->second, x, nbs_q_rank[x], nbs_q.size());
							nbs_q.erase(itq);
						}
						/*
						else {
							printf("Error:\n");
							for (auto itx = nbs_q.begin(); itx != nbs_q.end(); ++itx) {
								printf("(%ld,%f) ", itx->first, itx->second);
								if ((itx->first == x) && (itx->second == itqr->second)) printf("X ");
							}
							printf("\n");
							//exit(1);
						}*/
					}
					//else printf("Not found in rank map\n");
					
					knin = kntot = 0.;
					for (j=0; j<al[x].size(); ++j) {
						if (has_node[al[x][j].first]) knin += al[x][j].second;
						kntot += al[x][j].second;
					}
					knin *= 2.;
					rank = knin/std::pow(static_cast<double>(kntot),a);
					//printf("Insert %ld %f\n", x, rank);
					nbs_q.insert(std::make_pair(x,rank));
					nbs_q_rank[x] = rank;
					 
					/*
					// Also update the current queue
					knin = kntot = 0.;
					for (j=0; j<al[x].size(); ++j) {
						if (has_node[al[x][j].first]) knin += al[x][j].second;
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
			/*
			// Update overlapping communities map
			ncoms[n].items(other_coms);
			for (auto it = other_coms.begin(); it != other_coms.end(); ++it) {
				if (*it == cid) continue;
				++ovn_nodes[*it];
				// If potential overlap beyond threshold
				if (ovn_nodes[*it] >= this->merge_th * com.size()) {
					long oth_cid = *it, nb_ovn = 0;
					// Check the number of nodes actually present in the other community
					for (auto itcom = com.begin(); itcom != com.end(); ++itcom)
						if (ncoms[*itcom].find(oth_cid)) ++nb_ovn;
					// If enough overlapping nodes
					if (nb_ovn >= this->merge_th * com.size()) {
						// Clean bit vector
						for (auto nit = com.begin(); nit != com.end(); ++nit) has_node[*nit].flip();
						// Self-destroy if fully encompassed
						if (nb_ovn == com.size()) this->clear_com(cid, com, ncoms);
						// Otherwise sort
						else std::sort(com.begin(), com.end());
						// Terminate the current thread and return the id of the community to merge with
						return oth_cid;
					}
					// Otherwise update the number of overlapping nodes to the correct value
					else ovn_nodes[*it] = nb_ovn;
				}
			}*/
		}
	}
	//printf("MID\n");
	// If a change has been made to the community, check nodes
	if (changed) {
		// Set the maximum number of iterations allowed
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
					if (has_node[al[n][i].first])
						knin += al[n][i].second;
					kntot += al[n][i].second;
				}
				knin *= 2.;
				// If fitness is better without the node
				fit = (kin - knin) / std::pow(ktot - kntot, a);
				if ((fit > gfit) && this->isStillConnected(al, com, has_node, n, allocated_bitvector2)) {
					PrintDb("Remove %ld (f=%f > gf=%f)\n", n+1, fit, gfit);
					// Remove node
					com[c] = com.back(); com.pop_back();
					ncoms[n].erase(cid);
					has_node[n].flip();
					// Update weight in and total
					kin -= knin;
					ktot -= kntot;
					// Remove from the neighbour list some of the node's neighbours
					for (i=0; i<al[n].size(); ++i) {
						j = al[n][i].first;
						if (!has_node[j]) { //if (!ncoms[j].find(cid)) {
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
		}
		while (!done);
		// Update community node list (restore ordered list)
		std::sort(com.begin(), com.end());
	}
	// Clean bit vector
	for (auto nit = com.begin(); nit != com.end(); ++nit) has_node[*nit].flip();
	//printf("OUT\n");
	// Return true if some changes have been made to the community, false otherwise
	return changed;
}

void MSCD_LFK2::check(const ds::Graph::adjlist & al,
					  const long cid,
					  const std::vector<long> & com,
					  std::vector< toolkit::atomic_set<long> > & ncoms,
					  std::set< std::pair<long,long> > & to_merge
					  ) {
	std::map<long,long> ovn_nodes;
	std::vector<long> other_coms;
	for (auto nit = com.begin(); nit != com.end(); ++nit) {
		ncoms[*nit].items(other_coms);
		for (auto it = other_coms.begin(); it != other_coms.end(); ++it) {
			if (*it != cid) {
				++ovn_nodes[*it];
				// If enough overlapping nodes
				if (ovn_nodes[*it] >= this->merge_th * com.size()) {
					// Request merge
					to_merge.insert((cid<*it)?std::make_pair(cid,*it):std::make_pair(*it,cid));
					return;
				}
			}
		}
	}
}
	
void MSCD_LFK2::merge(const ds::Graph::adjlist & al, // adjacency list
					  const long cid1,
					  const long cid2,
					  std::vector<long> & com1, // community nodes
					  std::set<long> & com_nbs1, // community neighbours
					  double & kin1,
					  double & ktot1,
					  std::vector<long> & com2, // community nodes
					  std::set<long> & com_nbs2, // community neighbours
					  double & kin2,
					  double & ktot2,
					  std::vector<bool> & allocated_bitvector1,
					  std::vector<bool> & allocated_bitvector2
					  ) {
	/*
	// Check special cases when a community is empty (following self-destruction because of full overlap)
	if (com2.empty()) return;
	else if (com1.empty()) {
		com1.swap(com2);
		kin1 = kin2;
		ktot1 = ktot2;
		com_nbs1.swap(com_nbs2);
		return;
	}*/
	// Otherwise merge
	std::vector<bool> & c1_has_node = allocated_bitvector1, & is_ovn_node = allocated_bitvector2;
	// Set community 1 nodes
	for (long i=0; i<com1.size(); ++i) c1_has_node[com1[i]].flip();
	// Number of shared nodes between the two communities
	long n = this->nb_shared_nodes(com1,com2);
	// Buffer for merged community
	std::vector<long> tmp_com(com1.size()+com2.size()-n), ovn_nodes(n);
	// Merge and process each new node from c2 added to c1
	auto it1 = com1.begin(), it2 = com2.begin(), itk = tmp_com.begin(), ito = ovn_nodes.begin();
	while ((it1 != com1.end()) && (it2 != com2.end())) {
		if (*it1 <= *it2) {
			*itk++ = *it1;
			if (*it1 == *it2) {
				is_ovn_node[*it1].flip();
				*ito++ = *it1;
				++it2;
			}
			++it1;
		}
		else *itk++ = *it2++;
	}
	while (it1 != com1.end()) *itk++ = *it1++;
	while (it2 != com2.end()) *itk++ = *it2++;
	// Update kin, ktot and ncoms
	kin1 += kin2;
	ktot1 += ktot2;
	for (it2 = com2.begin(); it2 != com2.end(); ++it2) {
		if (is_ovn_node[*it2]) continue;
		// Update kin1
		for (auto itn = al[*it2].begin(); itn != al[*it2].end(); ++itn)
			if ((c1_has_node[itn->first]) && (!is_ovn_node[itn->first]))
				kin1 += 2.*itn->second;
	}
	for (ito = ovn_nodes.begin(); ito != ovn_nodes.end(); ++ito) {
		for (auto itn = al[*ito].begin(); itn != al[*ito].end(); ++itn) {
			if (is_ovn_node[itn->first])
				kin1 -= itn->second;
			ktot1 -= itn->second;
		}
	}
	// Update neighbour nodes and reset c1 bit vector
	for (it1 = com1.begin(); it1 != com1.end(); ++it1) {
		com_nbs2.erase(*it1);
		c1_has_node[*it1].flip();
	}
	for (it2 = com2.begin(); it2 != com2.end(); ++it2) com_nbs1.erase(*it2);
	for (auto it = com_nbs2.begin(); it != com_nbs2.end(); ++it) com_nbs1.insert(*it);
	// Put new community as c1
	com1.swap(tmp_com);
	// Unset overlapping nodes
	for (ito = ovn_nodes.begin(); ito != ovn_nodes.end(); ++ito) is_ovn_node[*ito].flip();
	// Clear com2
	com2.clear();
#ifdef __DEBUG__
	double dkin, dktot;
	this->DbCheckKs(al, com1, dkin, dktot);
	if ((dkin != kin1) || (dktot != ktot1))
		fprintf(stderr, "AM: Incorrect kin/ktot values: kin=%f (%f), ktot=%f (%f)\n", kin1, dkin, ktot1, dktot);
#endif
}

#ifdef __DEBUG__
void MSCD_LFK2::DbCheckKs(const ds::Graph::adjlist & al, const std::vector<long> & com, double & kin, double & ktot) const {
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

#endif

/*
 // Set of communities to delete at the end of the current loop
 std::set<long> to_delete;
 
 // Merge communities
 if (!to_merge.empty()) {
 std::set<long> com_proc;
 std::map<long,long> com_chg;
 long c1, c2;
 // While there are communities to merge
 if (bitvectors.size() < to_merge.size()) bitvectors.resize(to_merge.size());
 while (!to_merge.empty()) {
 threads.clear();
 thread_merge_data.clear();
 com_proc.clear();
 com_chg.clear();
 // Iterate through pairs of communities to merge
 auto it = to_merge.begin();
 while ((it != to_merge.end()) && (threads.size() < nb_threads)) {
 c1 = it->first;
 c2 = it->second;
 // In case this happens
 if (c1 == c2) to_merge.erase(it++);
 // Start new merge threads when the communities are not modified in others
 else if ((com_proc.find(c1) == com_proc.end()) && (com_proc.find(c2) == com_proc.end())) {
 PrintDb("Merge communities %ld and %ld\n", c1+1, c2+1);
 to_merge.erase(it++);
 com_proc.insert(c1);
 com_proc.insert(c2);
 to_delete.insert(c2);
 com_chg[c2] = c1;
 // Launch merge thread
 thread_merge_data.push_back(new ThreadMergeData(al, c1, c2, ncoms, coms[c1], coms_nbs[c1], kin[c1], ktot[c1], coms[c2], coms_nbs[c2], kin[c2], ktot[c2], bitvectors[threads.size()].first, bitvectors[threads.size()].second));
 threads.push_back(std::thread(thread_merge_func, std::ref(*this), std::ref(*thread_merge_data.back())));
 }
 // Merge cannot be done in parallel at the moment
 else ++it;
 }
 for (long t=0; t<threads.size(); ++t) {
 threads[t].join();
 delete thread_merge_data[t];
 }
 // Replace all references to c2 communities left with references to the c1 they joined
 std::set< std::pair<long,long> > new_to_merge;
 std::map<long,long>::iterator mit;
 for (it = to_merge.begin(); it != to_merge.end(); ++it) {
 mit = com_chg.find(it->first);
 c1 = (mit != com_chg.end()) ? mit->second : it->first;
 mit = com_chg.find(it->second);
 c2 = (mit != com_chg.end()) ? mit->second : it->second;
 new_to_merge.insert(std::make_pair(c1,c2));
 }
 to_merge.swap(new_to_merge);
 }
 }
 // Remove empty communities
 for (auto it = to_delete.rbegin(); it != to_delete.rend(); ++it) {
 coms[*it].swap(coms.back());			coms.pop_back();
 kin[*it] = kin.back();					kin.pop_back();
 ktot[*it] = ktot.back();				ktot.pop_back();
 coms_nbs[*it].swap(coms_nbs.back());	coms_nbs.pop_back();
 }
 */