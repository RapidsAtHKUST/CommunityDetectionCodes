


inline double log_zero(double a) {

	if(a<=0)
		return -1e20;
	else
		return log(a);

}




class oslomnet_evaluate : public oslomnet_louvain {
	
	
public:
	
	oslomnet_evaluate(deque<deque<int> > & b, deque<deque<pair<int, double> > > & c, deque<int> & d): oslomnet_louvain() { set_graph(b,c,d); set_maxbord(); set_changendi_cum(); };
	oslomnet_evaluate(string a): oslomnet_louvain() { set_graph(a); set_maxbord(); set_changendi_cum(); };
	oslomnet_evaluate(map<int, map<int, pair<int, double> > > & A) : oslomnet_louvain() { set_graph(A);  set_maxbord(); set_changendi_cum(); };
	~oslomnet_evaluate(){};
	

	
	double CUP_both(const deque<int> & _c_, deque<int> & gr_cleaned, int);
	double CUP_check(const deque<int> & _c_, deque<int> & gr_cleaned, int);
	double group_inflation(const deque<int> & _c_, deque<int> & gr_cleaned, int);
	
	
private:
	
	
	void erase_cgroup(int wnode);
	void insert_cgroup(int wnode);
	bool erase_the_worst(int & wnode);

	int set_maxbord();
	void set_cgroup_and_neighs(const deque<int> & G);
	double all_external_test(int kout_g, int tm, int Nstar, int nneighs, const double & max_r_one, const double & maxr_two, deque<int> & gr_cleaned, bool only_c, weighted_tabdeg & previous_tab_c);
	double cup_on_list(cup_data_struct & a, deque<int> & gr_cleaned);
	void get_external_scores(weighted_tabdeg & ne_, cup_data_struct & fitness_label_to_sort, int kout_g, int tm, int Nstar, int nneighs, const double & max_r, bool only_c, weighted_tabdeg & previous_tab_c);

	
	
	double CUP_runs(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev, deque<int> & gr_cleaned, bool only_c, int runs);
	void initialize_for_evaluation(const deque<int> & _c_, weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev);
	void initialize_for_evaluation(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev);
	double partial_CUP(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev, deque<int> & gr_cleaned, bool only_c);
	void set_changendi_cum();
	
	void insertion(int changendi);
	bool insert_the_best();
	
	double CUP_iterative(const deque<int> & _c_, deque<int> & gr_cleaned, int);
	double CUP_search(const deque<int> & _c_, deque<int> & gr_cleaned, int);
	
	
	/* DATA ***************************************************/

	double  max_r_bord;								// this is the maximum r allowed for the external nodes (we don't want to look at all the graph, it would take too long)
	int maxb_nodes;									// this is the maximum number of nodes allowed in the border (similar as above)
	deque<double> changendi_cum;					// this is the cumulative distribution of the number of nodes to add to the cluster in the group_inflation function 
	
	// ************* things to update *************************
	weighted_tabdeg cgroup;									//*
	weighted_tabdeg neighs;									//*
															//*
	int kin_cgroup;											//*
	int ktot_cgroup;										//*
	/*********************************************************/

	
};






void oslomnet_evaluate::set_changendi_cum() {
	
	if(dim!=0 && oneM!=0) {
		int flat_until= cast_int(oneM/dim * 3);
		flat_until=min(dim/2, flat_until);
		
		int max_p= max(paras.CUT_Off, flat_until);		// this is something which might be optimized
		max_p=min(dim/2, max_p);
		
		
		powerlaw(max_p, flat_until+1, 3, changendi_cum);
		deque<double> distr;
		distribution_from_cumulative(changendi_cum, distr);
		double ac=1;
		
		if(distr.size()>0)
			ac=distr[0];
		
		
		for(int i=0; i<flat_until; i++)
			distr.push_front(ac);
		
		
		normalize_one(distr);
		cumulative_from_distribution(changendi_cum, distr);
	}

}


int oslomnet_evaluate::set_maxbord() {
	
	max_r_bord=paras.maxbg_ordinary;
	maxb_nodes=paras.maxborder_nodes;
		
	return 0;


}




void oslomnet_evaluate::erase_cgroup(int wnode) {
		
	
	map<int, facts>::iterator itm= cgroup.lab_facts.find(wnode);
	if(itm!=cgroup.lab_facts.end()) {

		int kp = itm->second.internal_degree;
		int kt = itm->second.degree;
		double mtlw= itm->second.minus_log_total_wr;

		kin_cgroup-= 2 * kp;
		ktot_cgroup-=kt;
		int kout_g= ktot_cgroup - kin_cgroup;
		int tm= oneM - ktot_cgroup;

		
		
		double fi= compute_global_fitness_ofive(kp, kout_g, tm, kt, mtlw, neighs.size()+1, dim - cgroup.size() +1);
		neighs.edinsert(wnode, kp, kt, mtlw, fi);
		
		cgroup.erase(wnode);

		deque<int> tobe;
		for(int i=0; i<vertices[wnode]->links->size(); i++) {
			
			
			
			if(cgroup.update_group(vertices[wnode]->links->l[i], -vertices[wnode]->links->w[i].first, -vertices[wnode]->links->w[i].second, 
								   dim - cgroup.size(), neighs.size(), kout_g, tm, vertices[vertices[wnode]->links->l[i]]->stub_number, tobe)==false)
				neighs.update_neighs(vertices[wnode]->links->l[i], -vertices[wnode]->links->w[i].first, -vertices[wnode]->links->w[i].second, 
									dim - cgroup.size(), kout_g, tm, vertices[vertices[wnode]->links->l[i]]->stub_number);
		
		}
		
		for(int i=0; i<int(tobe.size()); i++)
			erase_cgroup(tobe[i]);

	}

}



bool oslomnet_evaluate::erase_the_worst(int & wnode) {
	
	
	
	// this function is to look for the worst node in cgroup and to erase it
	
	
	int Nstar= dim - cgroup.size();
	int nn= neighs.size();
	int kout_g = ktot_cgroup - kin_cgroup;
	int tm= oneM - ktot_cgroup;
	
	double wf;
	cgroup.worst_node(wnode, wf, kout_g, Nstar, nn, tm);

	
	if(cgroup.size()==0) {		
		return false;
	}
	
		
	erase_cgroup(wnode);

		
	return true;

}





void oslomnet_evaluate::insert_cgroup(int wnode) {

		
	// this function is to insert wnode into cgroup  updating all the system, neighs - kin_cgroup - ktot_cgroup
	// it needs to be differenciated between weighted and unweighted
	
	
	int kp, kt;
	double mtlw;
	
	{
	
		map<int, facts>::iterator itm= neighs.lab_facts.find(wnode);
		if(itm!=neighs.lab_facts.end()) {
			kp = itm->second.internal_degree;
			kt = itm->second.degree;
			mtlw= itm->second.minus_log_total_wr;
		
		} else {
			
			kp=0;
			kt=vertices[wnode]->stub_number;
			mtlw=0;
			
		}
		
	}	
		
	
	int kout_g= ktot_cgroup - kin_cgroup;
	int tm= oneM - ktot_cgroup;
	
	
	
	double fi= compute_global_fitness_ofive(kp, kout_g, tm, kt, mtlw, neighs.size(), dim - cgroup.size());
	
	kin_cgroup+= 2 * kp;
	ktot_cgroup+=kt;
	kout_g= ktot_cgroup - kin_cgroup;
	tm= oneM - ktot_cgroup;

	
	
	
	cgroup.edinsert(wnode, kp, kt, mtlw, fi);
	neighs.erase(wnode);
	
	deque<int> tobe;
	
	for(int i=0; i<vertices[wnode]->links->size(); i++) {
		
		if(cgroup.update_group(vertices[wnode]->links->l[i], vertices[wnode]->links->w[i].first, vertices[wnode]->links->w[i].second, 
							   dim - cgroup.size(), neighs.size(), kout_g, tm, vertices[vertices[wnode]->links->l[i]]->stub_number, tobe)==false)
			neighs.update_neighs(vertices[wnode]->links->l[i], vertices[wnode]->links->w[i].first, vertices[wnode]->links->w[i].second, 
								dim - cgroup.size(), kout_g, tm, vertices[vertices[wnode]->links->l[i]]->stub_number);
		
	}
	
	
}



void oslomnet_evaluate::set_cgroup_and_neighs(const deque<int> & G) {

	// this function initially sets the data structures for the group and its neighbors
	
	
	kin_cgroup=0;
	ktot_cgroup=0;
	cgroup.clear();
	neighs.clear();
	
	
	for(int i=0; i<int(G.size()); i++)
		insert_cgroup(G[i]);

}






double oslomnet_evaluate::cup_on_list(cup_data_struct & a, deque<int> & gr_cleaned) {
	
	
	
	
	int Nstar;
	if(paras.weighted)
		Nstar=neighs.size();
	else
		Nstar=dim-cgroup.size();

	double critical_xi= -log(1-paras.threshold)/fitted_exponent(Nstar);
	int pos=Nstar;
	
	
	int until=-1;								// until tells how many nodes should be included into the cluster - actually the number of good nodes are (until +1)
	double probability_a, probability_b;		// these are the two extremes of a possible good node I could have found
	double c_min=1;								// this is the score we give to the border we are evaluating here
	//cout<<"critical_xi: "<<critical_xi<<" --------------------------------------- "<<neighs.size()<<" cgroup "<<cgroup.size()<<endl<<endl<<endl;
	
	cup_data_struct :: iterator itl=a.begin();
	
	
	while(itl!=a.end()) {
		
		
		double c_pos=order_statistics_left_cumulative(Nstar, pos, itl->first);
		
		//cout<<"position .... "<<pos<<" "<<order_statistics_left_cumulative(Nstar, pos, itl->first + itl->second.second)<<" >>AAA<< "<<Nstar<<" +++ "<<itl->first + itl->second.second<<" "<<itl->first - itl->second.second<<endl;
		c_min=min(c_pos, c_min);
		
		if(c_pos<critical_xi) {		
			
			/*	
			this is the basic condition of the order statistics test
			it's saying: look, this guy (itl->second.first) has an average fitness (itl->first) 
			whose order_statistics_left_cumulative is below the threshold 
			*/
			
		
			if(until==-1) {			// this node is the first node to be below the threshold
				
				until= Nstar-pos;
				c_min=c_pos;
				probability_a=itl->first - itl->second.second;
				probability_b=itl->first + itl->second.second;
				
			} else {				
			
				/* 
				the previous node was already below the threshold. 
				In this case I need to know if I should stop now or go on.
				The condition is related to the probability_to_overtake the previous guy
				*/
				
				
				double probability_to_overtake= compare_r_variables(probability_a, probability_b, itl->first - itl->second.second, itl->first + itl->second.second);
				
				
							
				if(probability_to_overtake>0.4999) {		/*preliminary check: this node is basically equivalent to the previous guy, I consider it good*/
				
				
					until= Nstar-pos;
					c_min=c_pos;
					probability_a=itl->first - itl->second.second;
					probability_b=itl->first + itl->second.second;
				
				
				} else {
					
					/*now I need to compute the bootstrap probability that the previous guy would have stopped the process*/ 
					
					
				
					if(   (probability_to_overtake==0)   ||  ((1.-probability_to_overtake) * compute_probability_to_stop(probability_a, probability_b, critical_xi, Nstar, pos+1)> 0.5001)) {
						if(equivalent_check_gather(a, until, probability_a, probability_b, Nstar, critical_xi))
							break;
					
					}
					until= Nstar-pos;
					c_min=c_pos;
					probability_a=itl->first - itl->second.second;
					probability_b=itl->first + itl->second.second;

				
				}
			
			}
			
			
		
		} else {		/* this node is not below the threshold */


			if(until!=-1) {		/* this means that this node is not good and the previous one was good. So, I stop here */
				if(equivalent_check_gather(a, until, probability_a, probability_b, Nstar, critical_xi))
					break;
			}
			
		
		
		}
		
		--pos;
		++itl;
	
	}
	
	
	// equalizer check
	// this check is important to see if the procedure stopped just because there were a lot of equivalents nodes

	
	if(until!=-1 && itl==a.end())
		equivalent_check_gather(a, until, probability_a, probability_b, Nstar, critical_xi);
	
	
		
	
	// inserting nodes in gr_cleaned

		
	int nodes_added=-1;
	itl=a.begin();

	while(itl!=a.end()) {
	
		if(nodes_added==until)
			break;
		
		gr_cleaned.push_back(itl->second.first);
		++itl;
		++nodes_added;
		
	}
	
	return pron_min_exp(Nstar, c_min);

}


double oslomnet_evaluate::all_external_test(int kout_g, int tm, int Nstar, int nneighs, const double & max_r_one, const double & maxr_two, deque<int> & gr_cleaned, bool only_c, weighted_tabdeg & previous_tab_c) {
	
	
	double max_r=min(max_r_one, maxr_two);
	
	cup_data_struct fitness_label_to_sort;
	
	get_external_scores(neighs, fitness_label_to_sort, kout_g, tm, Nstar, nneighs, max_r, only_c, previous_tab_c);

	return cup_on_list(fitness_label_to_sort, gr_cleaned);

}




void oslomnet_evaluate::initialize_for_evaluation(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev) {
	

	int Nstar= dim - cgroup.size();
	int nn= neighs.size();
	int kout_g = ktot_cgroup - kin_cgroup;
	int tm= oneM - ktot_cgroup;
	
	previous_tab_c.set_and_update_group(Nstar, nn, kout_g, tm, cgroup);
	previous_tab_n.set_and_update_neighs(Nstar, nn, kout_g, tm, neighs);
	kin_cgroup_prev=kin_cgroup;
	ktot_cgroup_prev=ktot_cgroup;

}





void oslomnet_evaluate::initialize_for_evaluation(const deque<int> & _c_, weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev) {
	
	set_cgroup_and_neighs(_c_);	

	int Nstar= dim - cgroup.size();
	int nn= neighs.size();
	int kout_g = ktot_cgroup - kin_cgroup;
	int tm= oneM - ktot_cgroup;
	
	previous_tab_c.set_and_update_group(Nstar, nn, kout_g, tm, cgroup);
	previous_tab_n.set_and_update_neighs(Nstar, nn, kout_g, tm, neighs);
	kin_cgroup_prev=kin_cgroup;
	ktot_cgroup_prev=ktot_cgroup;

}




double oslomnet_evaluate::partial_CUP(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev, deque<int> & border_group, bool only_c) {
	
	
	// still there is some stochasticity due to possible ties
		
	/*	previous_stuff is the module-stuff before the CUP (Clean Up Procedure)
		cgroup + border_group is the module cleaned					*/
	
	
	border_group.clear();
	
	cgroup._set_(previous_tab_c);
	neighs._set_(previous_tab_n);
	kin_cgroup=kin_cgroup_prev;
	ktot_cgroup=ktot_cgroup_prev;
	
	if(cgroup.size()==dim) {
		return 1;	
	}
	
	double bscore=1;
	while (true) {
		
		bscore= all_external_test(ktot_cgroup - kin_cgroup, oneM - ktot_cgroup, dim - cgroup.size(), neighs.size(), maxb_nodes/double(dim-cgroup.size()), max_r_bord, border_group, only_c, previous_tab_c);
		
		if(border_group.size()>0)
			break;
		
		if(cgroup.size()==0)
			break;
		
		int wnode;
		erase_the_worst(wnode);
	}
	
	
	return bscore;	
}



double oslomnet_evaluate::CUP_runs(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev, deque<int> & gr_cleaned, bool only_c, int number_of_runs) {
	
	
	/* this if statemets are here to speed up the program if there are big clusters */
	if(previous_tab_c.size()>100000)
		number_of_runs=3;
	else if(previous_tab_c.size()>10000)
		number_of_runs=5;
	else if(previous_tab_c.size()>1000)
		number_of_runs=10;

	gr_cleaned.clear();

	if(previous_tab_c.size()==0)
		return 1;

	int max_gr_size=0;
	double bscore=1;
	
	int good_runs=0;
	
	for(int i=0; i<number_of_runs; i++) {
		
		deque<int> gr_run_i;

		double score_i= partial_CUP(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev, gr_run_i, only_c);
						
		if(cgroup.size()+ int(gr_run_i.size())> max_gr_size) {
			
			bscore=score_i;
			cgroup.set_deque(gr_cleaned);
			for(int j=0; j<int(gr_run_i.size()); j++)
				gr_cleaned.push_back(gr_run_i[j]);
			
			max_gr_size=gr_cleaned.size();
			sort(gr_cleaned.begin(), gr_cleaned.end());
		}
		
		if(gr_run_i.size()>0) {
			++good_runs;
			if(good_runs>=0.55 * number_of_runs)
				return bscore;
		}	
	}
		
	if(good_runs<0.55*number_of_runs) {
		gr_cleaned.clear();
		bscore+=paras.threshold;
		bscore=min(1., bscore);
	}
	
	return bscore;

}





double oslomnet_evaluate::CUP_check(const deque<int> & _c_, deque<int> & gr_cleaned, int number_of_runs =  paras.clean_up_runs) {
	
	
	/*_c_ is the module to clean up and gr_cleaned is the result */
	
	
	weighted_tabdeg previous_tab_c;
	weighted_tabdeg previous_tab_n;
	int kin_cgroup_prev;
	int ktot_cgroup_prev;
	double bscore;
	
	initialize_for_evaluation(_c_, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev);
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev, gr_cleaned, true, number_of_runs);
	
	

	return bscore;
}






double oslomnet_evaluate::CUP_search(const deque<int> & _c_, deque<int> & gr_cleaned, int number_of_runs =  paras.clean_up_runs) {
	
	
	/*_c_ is the module to clean up and gr_cleaned is the result */
	
	
	weighted_tabdeg previous_tab_c;
	weighted_tabdeg previous_tab_n;
	int kin_cgroup_prev;
	int ktot_cgroup_prev;
	double bscore;
	
	initialize_for_evaluation(_c_, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev);
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev, gr_cleaned, false, number_of_runs);
	

	return bscore;
}



double oslomnet_evaluate::CUP_both(const deque<int> & _c_, deque<int> & gr_cleaned, int number_of_runs =  paras.clean_up_runs) {
	
	
	/*_c_ is the module to clean up and gr_cleaned is the result */
	
	
	weighted_tabdeg previous_tab_c;
	weighted_tabdeg previous_tab_n;
	int kin_cgroup_prev;
	int ktot_cgroup_prev;
	double bscore;
	
	initialize_for_evaluation(_c_, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev);
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev, gr_cleaned, false, number_of_runs);
	
	initialize_for_evaluation(gr_cleaned, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev);
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev, gr_cleaned, true, number_of_runs);		/*this "true" means I can only look at nodes in previous_tab_c*/

	return bscore;
}




double oslomnet_evaluate::CUP_iterative(const deque<int> & _c_, deque<int> & gr_cleaned, int number_of_runs =  paras.clean_up_runs) {
	
	double bs= CUP_both(_c_, gr_cleaned, number_of_runs);
	
	int stopp=0;
	
	do {
		
		deque<int> _c_temp = gr_cleaned;

		bs = CUP_search(_c_temp, gr_cleaned, number_of_runs);
		++stopp;
				
		if(stopp==paras.iterative_stopper)
			break;
	
	} while (gr_cleaned.size()>_c_.size());

	
	return bs;

}




bool oslomnet_evaluate::insert_the_best() {
	
	
	int Nstar= dim - cgroup.size();
	int nn= neighs.size();
	int kout_g = ktot_cgroup - kin_cgroup;
	int tm= oneM - ktot_cgroup;
	
	double lowest_r;
	int benode;
	neighs.best_node(benode, lowest_r, kout_g, Nstar, nn, tm);
	
	if(benode==-1)
		return false;
			
	insert_cgroup(benode);
	
	return true;

}



void oslomnet_evaluate::insertion(int changendi) {

	for(int i=0; i<changendi; i++)
		insert_the_best();

}





double oslomnet_evaluate::group_inflation(const deque<int> & _c_, deque<int> & gr_cleaned, int number_of_runs=paras.inflate_runs) {
		
	
	
	/* preliminary check */
	double bscore=CUP_iterative(_c_, gr_cleaned);
	
	if(gr_cleaned.size()>0) {
		return bscore;
	}
	/* preliminary check */
		
	//cout<<"group inflating... "<<endl;
	weighted_tabdeg _c_tab_c;
	weighted_tabdeg _c_tab_n;
	int kin_cgroup_c;
	int ktot_cgroup_c;
	
	initialize_for_evaluation(_c_, _c_tab_c, _c_tab_n, kin_cgroup_c, ktot_cgroup_c);
	

	weighted_tabdeg previous_tab_c;
	weighted_tabdeg previous_tab_n;
	int kin_cgroup_prev;
	int ktot_cgroup_prev;
	
	
	int stopper=0;
	while(true) {
		

		
		cgroup._set_(_c_tab_c);
		neighs._set_(_c_tab_n);
		kin_cgroup=kin_cgroup_c;
		ktot_cgroup=ktot_cgroup_c;
		
				
		int changendi=lower_bound(changendi_cum.begin(), changendi_cum.end(), ran4()) - changendi_cum.begin() + 1;
		changendi=min(changendi, neighs.size());
		insertion(changendi);
		
			
		if(cgroup.size()==dim)
			return 1;
		
		
		/*here it make a CUP_search using c_group with the nodes added*/
		initialize_for_evaluation(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev);
		CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev, gr_cleaned, false, number_of_runs);
		
		
		if(gr_cleaned.size()>0) {
			
			/*the first clean up passed. now it makes the CUP_check*/
			initialize_for_evaluation(gr_cleaned, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev);
			bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev, gr_cleaned, true, paras.clean_up_runs);
			
			//cout<<"exiting... "<<gr_cleaned.size()<<endl;
			if(gr_cleaned.size()>0) {
				return bscore;
			}
		}
		++stopper;
		if(stopper==paras.inflate_stopper)
			break;
			
	}
	
	//cout<<"bad group"<<endl;
	
	return 1;
	
}


void oslomnet_evaluate::get_external_scores(weighted_tabdeg& neighs, cup_data_struct & fitness_label_to_sort, int kout_g, int tm, int Nstar, int nneighs, const double & max_r, bool only_c, weighted_tabdeg & previous_tab_c) {



	multimap<double, int>:: iterator bit= neighs.fitness_lab.begin();
	
	
	int counter=0;
	
	while(bit!=neighs.fitness_lab.end()) {
		map<int, facts> :: iterator itm= neighs.lab_facts.find(bit->second);				
		double interval;
		double F= compute_global_fitness(itm->second.internal_degree, kout_g, tm, itm->second.degree, itm->second.minus_log_total_wr, nneighs, Nstar, interval);
		
		
		if(F>max_r) {
			
			/*if(only_c == false || previous_tab_c.is_internal(itm->first))		
				cout<<"no node: "<<vertices[itm->first]->id_num<<"  "<<itm->second.internal_degree<<" / "<< itm->second.degree<<" fitness: "<<F<<endl;*/ 
			
			counter++;
			if(counter>num_up_to)
				break;
		}
		else {
			
			/*if(only_c == false || previous_tab_c.is_internal(itm->first))
				cout<<"node: "<<"  "<<itm->second.internal_degree<<" / "<< itm->second.degree<<" fitness: "<<F<<" "<<interval<<endl;*/
			
			if(only_c == false || previous_tab_c.is_internal(itm->first))
				fitness_label_to_sort.insert(make_pair(F, make_pair(itm->first, interval)));
		}
		
		bit++;	
	}
	


}

