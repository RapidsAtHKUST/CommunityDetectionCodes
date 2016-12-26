


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
	int try_to_assign_homeless_help(module_collection & module_coll, map<int, deque<int> > & to_check);
	
private:
	
	double CUP_iterative(const deque<int> & _c_, deque<int> & gr_cleaned, int);
	double CUP_search(const deque<int> & _c_, deque<int> & gr_cleaned, int);
	void erase_cgroup(int wnode);
	void insert_cgroup(int wnode);
	bool erase_the_worst(int & wnode);

	int set_maxbord();
	void set_cgroup_and_neighs(const deque<int> & G);
	double all_external_test(int kout_g_in, int tmin, int kout_g_out, int tmout, 
							int Nstar, int nneighs, const double & max_r_one, const double & maxr_two, deque<int> & gr_cleaned, bool only_c, weighted_tabdeg & previous_tab_c);
	double cup_on_list(cup_data_struct & a, deque<int> & gr_cleaned);
	void get_external_scores(weighted_tabdeg & neighs, cup_data_struct & fitness_label_to_sort, int kout_g_in, int tmin, int kout_g_out, int tmout, 
							int Nstar, int nneighs, const double & max_r, bool only_c, weighted_tabdeg & previous_tab_c);

	
	
	double CUP_runs(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev_in, int ktot_cgroup_prev_out, deque<int> & gr_cleaned, bool only_c, int number_of_runs);
	void initialize_for_evaluation(const deque<int> & _c_, weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev_in, int & ktot_cgroup_prev_out);
	void initialize_for_evaluation(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev_in, int & ktot_cgroup_prev_out);
	double partial_CUP(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev_in, int ktot_cgroup_prev_out, deque<int> & border_group, bool only_c);
	void set_changendi_cum();
	
	void insertion(int changendi);
	bool insert_the_best();

	
	
	
	/* DATA ***************************************************/

	double  max_r_bord;								// this is the maximum r allowed for the external nodes (we don't want to look at all the graph, it would take too long)
	int maxb_nodes;									// this is the maximum number of nodes allowed in the border (similar as above)
	deque<double> changendi_cum;					// this is the cumulative distribution of the number of nodes to add to the cluster in the group_inflation function 
	
	// ************* things to update *************************
	weighted_tabdeg cgroup;									//*
	weighted_tabdeg neighs;									//*
															//*
	int kin_cgroup;											//*
	int ktot_cgroup_in;										//*
	int ktot_cgroup_out;									//*
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
		
		
		
		
		
		int kpin = itm->second.internal_indegree;
		int ktin = itm->second.indegree;
		int kpout = itm->second.internal_outdegree;
		int ktout = itm->second.outdegree;
		double mtlwin= itm->second.minus_log_total_wrin;
		double mtlwout= itm->second.minus_log_total_wrout;


		kin_cgroup-=  kpin + kpout;
		ktot_cgroup_out-=  ktout;
		ktot_cgroup_in-=  ktin;
		
		
		int kout_g_in= ktot_cgroup_in - kin_cgroup;
		int kout_g_out= ktot_cgroup_out - kin_cgroup;
		
		int tmin= oneM - ktot_cgroup_in;
		int tmout= oneM - ktot_cgroup_out;

		
		double fi= compute_global_fitness_ofive(kpin, kout_g_in, kpout, kout_g_out, tmin, tmout, ktin, ktout, mtlwin, mtlwout, neighs.size()+1, dim - cgroup.size() +1);
		neighs.edinsert(wnode, kpin, kpout, ktin, ktout, mtlwin, mtlwout, fi);
		
		//cout<<"node erased: "<<id_of(wnode)<<endl;
		cgroup.erase(wnode);


		deque<int> tobe;
		pair <int, pair<int, double> > OPA;
	
		
		
		for(int i=0; i<vertices[wnode]->inlinks->size(); i++) {
			
			OPA = vertices[wnode]->outlinks->posweightof(vertices[wnode]->inlinks->l[i]);
			
			
			
			if(cgroup.update_group(vertices[wnode]->inlinks->l[i], -vertices[wnode]->inlinks->w[i].first, -OPA.second.first, -vertices[wnode]->inlinks->w[i].second, -OPA.second.second, 
						dim - cgroup.size(), neighs.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->inlinks->l[i]]->instub_number, vertices[vertices[wnode]->inlinks->l[i]]->outstub_number,tobe)==false)
				neighs.update_neighs(vertices[wnode]->inlinks->l[i], -vertices[wnode]->inlinks->w[i].first, -OPA.second.first, -vertices[wnode]->inlinks->w[i].second, -OPA.second.second, 
						dim - cgroup.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->inlinks->l[i]]->instub_number, vertices[vertices[wnode]->inlinks->l[i]]->outstub_number);
		
		}
		
		
		for(int i=0; i<vertices[wnode]->outlinks->size(); i++) {
		
		
			OPA = vertices[wnode]->inlinks->posweightof(vertices[wnode]->outlinks->l[i]);
		
			if(OPA.first==-1) if(cgroup.update_group(vertices[wnode]->outlinks->l[i], 0, -vertices[wnode]->outlinks->w[i].first, 0, -vertices[wnode]->outlinks->w[i].second,
						dim - cgroup.size(), neighs.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->outlinks->l[i]]->instub_number, vertices[vertices[wnode]->outlinks->l[i]]->outstub_number,tobe)==false)
				neighs.update_neighs(vertices[wnode]->outlinks->l[i], 0, -vertices[wnode]->outlinks->w[i].first, 0, -vertices[wnode]->outlinks->w[i].second,
						dim - cgroup.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->outlinks->l[i]]->instub_number, vertices[vertices[wnode]->outlinks->l[i]]->outstub_number);
		
		}

		
	
		
		for(UI i=0; i<tobe.size(); i++)
			erase_cgroup(tobe[i]);

		
		
		
	}

}



bool oslomnet_evaluate::erase_the_worst(int & wnode) {
	
	
	
	// this function is to look for the worst node in cgroup and to erase it
	
	
	int Nstar= dim - cgroup.size();
	int nn= neighs.size();
	int kout_g_in = ktot_cgroup_in - kin_cgroup;
	int kout_g_out = ktot_cgroup_out - kin_cgroup;
	int tmin= oneM - ktot_cgroup_in;
	int tmout= oneM - ktot_cgroup_out;
	
	double wf;
	
	
	cgroup.worst_node(wnode, wf, kout_g_in, kout_g_out, Nstar, nn, tmin, tmout);	
	
	if(cgroup.size()==0) {		
		return false;
	}
	

		
	erase_cgroup(wnode);

		
	return true;

}





void oslomnet_evaluate::insert_cgroup(int wnode) {

		
		
	/* this function is to insert benode into cgroup  updating all the system, neighs - kin_cgroup - ktot_cgroup */
	
		
	
	int kpin, ktin, kpout, ktout;
	double mtlwin, mtlwout;
	
	{
	
		map<int, facts>::iterator itm= neighs.lab_facts.find(wnode);
		if(itm!=neighs.lab_facts.end()) {
			
			
			kpin = itm->second.internal_indegree;
			ktin = itm->second.indegree;
			
			kpout = itm->second.internal_outdegree;
			ktout = itm->second.outdegree;

			mtlwin= itm->second.minus_log_total_wrin;
			mtlwout= itm->second.minus_log_total_wrout;
		
		
		
		} else {
			
			kpin=0;
			kpout=0;
			ktin=vertices[wnode]->instub_number;
			ktout=vertices[wnode]->outstub_number;
			mtlwin=0;
			mtlwout=0;
			
		}
		
	}	
	

	
	int kout_g_in= ktot_cgroup_in - kin_cgroup;
	int kout_g_out= ktot_cgroup_out - kin_cgroup;	
	int tmin= oneM - ktot_cgroup_in;
	int tmout= oneM - ktot_cgroup_out;
	

	double fi= compute_global_fitness_ofive(kpin, kout_g_in, kpout, kout_g_out, tmin, tmout, ktin, ktout, mtlwin, mtlwout, neighs.size(), dim - cgroup.size());
	
	kin_cgroup+=  kpin + kpout;
	ktot_cgroup_in+= ktin;
	ktot_cgroup_out+= ktout;
	kout_g_in= ktot_cgroup_in - kin_cgroup;
	kout_g_out= ktot_cgroup_out - kin_cgroup;	
	tmin= oneM - ktot_cgroup_in;
	tmout= oneM - ktot_cgroup_out;
	
	
	
	 
	cgroup.edinsert(wnode, kpin, kpout, ktin, ktout, mtlwin, mtlwout, fi);
	neighs.erase(wnode);
	
	
	
	deque<int> tobe;
	pair <int, pair<int, double> > OPA;
	
	for(int i=0; i<vertices[wnode]->inlinks->size(); i++) {
	
		OPA = vertices[wnode]->outlinks->posweightof(vertices[wnode]->inlinks->l[i]);
		
		//cout<<"------> "<<vertices[wnode]->inlinks->w[i].second<<" "<<OPA.second.second<<endl;
		
		if(cgroup.update_group(vertices[wnode]->inlinks->l[i], vertices[wnode]->inlinks->w[i].first, OPA.second.first, vertices[wnode]->inlinks->w[i].second,  OPA.second.second, 
					dim - cgroup.size(), neighs.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->inlinks->l[i]]->instub_number, vertices[vertices[wnode]->inlinks->l[i]]->outstub_number,tobe)==false)
			neighs.update_neighs(vertices[wnode]->inlinks->l[i], vertices[wnode]->inlinks->w[i].first, OPA.second.first, vertices[wnode]->inlinks->w[i].second, OPA.second.second, 
					dim - cgroup.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->inlinks->l[i]]->instub_number, vertices[vertices[wnode]->inlinks->l[i]]->outstub_number);
	
	}
	
	
	
	for(int i=0; i<vertices[wnode]->outlinks->size(); i++) {
		
		
		
		OPA = vertices[wnode]->inlinks->posweightof(vertices[wnode]->outlinks->l[i]);
		
		if(OPA.first==-1) if(cgroup.update_group(vertices[wnode]->outlinks->l[i], 0, vertices[wnode]->outlinks->w[i].first, 0, vertices[wnode]->outlinks->w[i].second,
					dim - cgroup.size(), neighs.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->outlinks->l[i]]->instub_number, vertices[vertices[wnode]->outlinks->l[i]]->outstub_number,tobe)==false)
			neighs.update_neighs(vertices[wnode]->outlinks->l[i], 0, vertices[wnode]->outlinks->w[i].first, 0, vertices[wnode]->outlinks->w[i].second,
					dim - cgroup.size(), kout_g_in, kout_g_out, tmin, tmout, vertices[vertices[wnode]->outlinks->l[i]]->instub_number, vertices[vertices[wnode]->outlinks->l[i]]->outstub_number);
	
	}

		
	
		
}



void oslomnet_evaluate::set_cgroup_and_neighs(const deque<int> & G) {

	
	kin_cgroup=0;
	ktot_cgroup_in=0;
	ktot_cgroup_out=0;
	cgroup.clear();
	neighs.clear();
	for(UI i=0; i<G.size(); i++)
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
				
				//cout<<"probability_to_overtake: "<<probability_to_overtake<<endl;
				
							
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


double oslomnet_evaluate::all_external_test(int kout_g_in, int tmin, int kout_g_out, int tmout, 
											int Nstar, int nneighs, const double & max_r_one, const double & maxr_two, deque<int> & gr_cleaned, bool only_c, weighted_tabdeg & previous_tab_c) {
	
	
	double max_r=min(max_r_one, maxr_two);
	
	cup_data_struct fitness_label_to_sort;
	
	get_external_scores(neighs, fitness_label_to_sort, kout_g_in, tmin, kout_g_out, tmout, Nstar, nneighs, max_r, only_c, previous_tab_c);

	return cup_on_list(fitness_label_to_sort, gr_cleaned);

}




void oslomnet_evaluate::initialize_for_evaluation(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev_in, int & ktot_cgroup_prev_out) {
	

	int Nstar= dim - cgroup.size();
	int nn= neighs.size();
	int kout_g_in= ktot_cgroup_in - kin_cgroup;
	int kout_g_out= ktot_cgroup_out - kin_cgroup;
		
	int tmin= oneM - ktot_cgroup_in;
	int tmout= oneM - ktot_cgroup_out;

	
	previous_tab_c.set_and_update_group(Nstar, nn, kout_g_out, tmout, kout_g_in, tmin, cgroup);
	previous_tab_n.set_and_update_neighs(Nstar, nn, kout_g_out, tmout, kout_g_in, tmin, neighs);
	
	kin_cgroup_prev=kin_cgroup;
	ktot_cgroup_prev_in=ktot_cgroup_in;
	ktot_cgroup_prev_out=ktot_cgroup_out;

}





void oslomnet_evaluate::initialize_for_evaluation(const deque<int> & _c_, weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int & kin_cgroup_prev, int & ktot_cgroup_prev_in, int & ktot_cgroup_prev_out) {
	
	set_cgroup_and_neighs(_c_);	

	int Nstar= dim - cgroup.size();
	int nn= neighs.size();
	int kout_g_in= ktot_cgroup_in - kin_cgroup;
	int kout_g_out= ktot_cgroup_out - kin_cgroup;
		
	int tmin= oneM - ktot_cgroup_in;
	int tmout= oneM - ktot_cgroup_out;

	previous_tab_c.set_and_update_group(Nstar, nn, kout_g_out, tmout, kout_g_in, tmin, cgroup);
	previous_tab_n.set_and_update_neighs(Nstar, nn, kout_g_out, tmout, kout_g_in, tmin, neighs);
	
	kin_cgroup_prev=kin_cgroup;
	ktot_cgroup_prev_in=ktot_cgroup_in;
	ktot_cgroup_prev_out=ktot_cgroup_out;
}




double oslomnet_evaluate::partial_CUP(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev_in, int ktot_cgroup_prev_out, deque<int> & border_group, bool only_c) {
	
	//draw_with_weight_probability("graph____");
	
	// still there is some stochasticity due to possible ties
		
	/*	previous_stuff is the module-stuff before the CUP (Clean Up Procedure)
		cgroup + border_group is the module cleaned					*/
	
	
	border_group.clear();
	
	
	
	
	//previous_tab_c.print_nodes(cout);
	
	
	
	cgroup._set_(previous_tab_c);
	neighs._set_(previous_tab_n);
	kin_cgroup=kin_cgroup_prev;
	ktot_cgroup_in=ktot_cgroup_prev_in;
	ktot_cgroup_out=ktot_cgroup_prev_out;
	
	if(cgroup.size()==dim) {
		return 1;	
	}
	
	
	
	
	
	
	
	double bscore=1;
	while (true) {
		
		
		
		
		bscore= all_external_test(ktot_cgroup_in - kin_cgroup, oneM - ktot_cgroup_in, ktot_cgroup_out - kin_cgroup, oneM - ktot_cgroup_out,
									dim - cgroup.size(), neighs.size(), maxb_nodes/double(dim-cgroup.size()), max_r_bord, border_group, only_c, previous_tab_c);
		
		
		//cout<<cgroup.size()<<endl;
		
		if(border_group.size()>0)
			break;
		
		
		if(cgroup.size()==0)
			break;
		
			
		int wnode;
		erase_the_worst(wnode);
	
		
				
	}
	
	
	return bscore;	
}



double oslomnet_evaluate::CUP_runs(weighted_tabdeg & previous_tab_c, weighted_tabdeg & previous_tab_n, int kin_cgroup_prev, int ktot_cgroup_prev_in, int ktot_cgroup_prev_out, 
									deque<int> & gr_cleaned, bool only_c, int number_of_runs) {

	
	
	/* this if statemets are here to speed up the program if there are big clusters */
	if(previous_tab_c.size()>100000)
		number_of_runs=3;
	else if(previous_tab_c.size()>10000)
		number_of_runs=5;
	else if(previous_tab_c.size()>1000)
		number_of_runs=10;

	
	
	gr_cleaned.clear();

	int max_gr_size=0;
	double bscore=1;
	
	int good_runs=0;
	
	if(previous_tab_c.size()==0)
		return 1;
	
	for(int i=0; i<number_of_runs; i++) {
		
		deque<int> gr_run_i;
		
		
		double score_i= partial_CUP(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out, gr_run_i, only_c);
						
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
	int ktot_cgroup_prev_in, ktot_cgroup_prev_out;
	double bscore;
	
	
	initialize_for_evaluation(_c_, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out);
	
	
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out, gr_cleaned, true, number_of_runs);
	
	

	return bscore;
}






double oslomnet_evaluate::CUP_search(const deque<int> & _c_, deque<int> & gr_cleaned, int number_of_runs =  paras.clean_up_runs) {
	
	
	/*_c_ is the module to clean up and gr_cleaned is the result */
	
	
	weighted_tabdeg previous_tab_c;
	weighted_tabdeg previous_tab_n;
	int kin_cgroup_prev;
	int ktot_cgroup_prev_in, ktot_cgroup_prev_out;
	double bscore;
	
	initialize_for_evaluation(_c_, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out);
	
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out, gr_cleaned, false, number_of_runs);
	

	return bscore;
}



double oslomnet_evaluate::CUP_both(const deque<int> & _c_, deque<int> & gr_cleaned, int number_of_runs =  paras.clean_up_runs) {
	
	
	/*_c_ is the module to clean up and gr_cleaned is the result */
	
	
	weighted_tabdeg previous_tab_c;
	weighted_tabdeg previous_tab_n;
	int kin_cgroup_prev;
	int ktot_cgroup_prev_in, ktot_cgroup_prev_out;
	double bscore;
	
	initialize_for_evaluation(_c_, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out);
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out, gr_cleaned, false, number_of_runs);
	
	
	initialize_for_evaluation(_c_, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out);
	bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out, gr_cleaned, true, number_of_runs);		/*this "true" means I can only look at nodes in previous_tab_c*/

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
	
	int kout_g_in = ktot_cgroup_in - kin_cgroup;
	int kout_g_out = ktot_cgroup_out - kin_cgroup;
	int tmin= oneM - ktot_cgroup_in;
	int tmout= oneM - ktot_cgroup_out;
	
	double lowest_r;
	int benode;
	neighs.best_node(benode, lowest_r, kout_g_in, kout_g_out, Nstar, nn, tmin, tmout);
	
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
	//cout<<"evaluating group of "<<_c_.size()<<" nodes"<<endl;
	double bscore=CUP_iterative(_c_, gr_cleaned);
	
	if(gr_cleaned.size()>0) {
		//cout<<"test passed, bscore: "<<bscore<<"  size: "<<gr_cleaned.size()<<endl;
		return bscore;
	}
	/* preliminary check */
		
	weighted_tabdeg _c_tab_c;
	weighted_tabdeg _c_tab_n;
	int kin_cgroup_c;
	int ktot_cgroup_c_in, ktot_cgroup_c_out;
	
	//print_id(_c_, cout);
	initialize_for_evaluation(_c_, _c_tab_c, _c_tab_n, kin_cgroup_c, ktot_cgroup_c_in, ktot_cgroup_c_out);
	

	weighted_tabdeg previous_tab_c;
	weighted_tabdeg previous_tab_n;
	int kin_cgroup_prev;
	int ktot_cgroup_prev_in, ktot_cgroup_prev_out;
	
	
	int stopper=0;
	while(true) {
		

		
		cgroup._set_(_c_tab_c);
		neighs._set_(_c_tab_n);
		kin_cgroup=kin_cgroup_c;
		ktot_cgroup_in=ktot_cgroup_c_in;
		ktot_cgroup_out=ktot_cgroup_c_out;
		
		/*
		deque<int> hhh;
		cgroup.set_deque(hhh);
		print_id(hhh, cout);
		//*/
		
		int changendi=lower_bound(changendi_cum.begin(), changendi_cum.end(), ran4()) - changendi_cum.begin() + 1;
		changendi=min(changendi, neighs.size());
		insertion(changendi);
		
		/*
		cout<<"... after "<<cgroup.size()<<endl;
		cgroup.set_deque(hhh);
		print_id(hhh, cout);//*/
		
		if(cgroup.size()==dim)
			return 1;
		
		
		/*here it make a CUP_search using c_group with the nodes added*/
		initialize_for_evaluation(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out);
		
		CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out, gr_cleaned, false, number_of_runs);
		
		
		if(gr_cleaned.size()>0) {
			
			/*the first clean up passed. now it makes the CUP_check*/
			
			initialize_for_evaluation(gr_cleaned, previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out);
			
			
			
			bscore=CUP_runs(previous_tab_c, previous_tab_n, kin_cgroup_prev, ktot_cgroup_prev_in, ktot_cgroup_prev_out, gr_cleaned, true, paras.clean_up_runs);

			
			if(gr_cleaned.size()>0) {
				//cout<<"test passed, bscore: "<<bscore<<"  size: "<<gr_cleaned.size()<<endl;
				return bscore;
			}
		}
		++stopper;
		if(stopper==paras.inflate_stopper)
			break;
			
	}
	
	
	return 1;
	
}


void oslomnet_evaluate::get_external_scores(weighted_tabdeg & neighs, cup_data_struct & fitness_label_to_sort, int kout_g_in, int tmin, int kout_g_out, int tmout, 
											int Nstar, int nneighs, const double & max_r, bool only_c, weighted_tabdeg & previous_tab_c) {



	multimap<double, int>:: iterator bit= neighs.fitness_lab.begin();
	
	
	int counter=0;
	
	while(bit!=neighs.fitness_lab.end()) {
		
		map<int, facts> :: iterator itm= neighs.lab_facts.find(bit->second);				
		double interval;
		
		
		/***************************************************************************************
		cout<<"boring check but somebody ...."<<endl;
		cout<<"node: "<<vertices[itm->first]->id_num<<endl;
		deque<int> hhh;
		cgroup.set_deque(hhh);
		cout<<"group"<<endl;
		print_id(hhh, cout);
		
		cout<<"ktot_cgroup_in: "<<ktot_cgroup_in<<" ktot_cgroup_out: "<<ktot_cgroup_out<<" kin_cgroup: "<<kin_cgroup<<endl;
		cout<<"internal in out  "<<itm->second.internal_indegree<<" "<<itm->second.internal_outdegree<<endl;
		
		
		*-**************************************************************************************/

		
		double F= compute_global_fitness(itm->second.internal_indegree, kout_g_in, itm->second.internal_outdegree, kout_g_out, tmin, tmout, 
										itm->second.indegree, itm->second.outdegree, itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, nneighs, Nstar, interval);
		
		//cout<<"tmin: "<<tmin<<" tmout "<<tmout<<" "<<oneM<<" ... "<<oneM - ktot_cgroup_out<<endl;
		//cout<<"------------------------------------------------------------------------------------"<<endl;
		if(F>max_r) {
			
			counter++;
			if(counter>num_up_to)
				break;
		}
		else {
						
			if(only_c == false || previous_tab_c.is_internal(itm->first))
				fitness_label_to_sort.insert(make_pair(F, make_pair(itm->first, interval)));
		}
		
		bit++;	
	}
	


}




int oslomnet_evaluate::try_to_assign_homeless_help(module_collection & module_coll, map<int, deque<int> > & to_check) {
	
	
	
	to_check.clear();
	module_coll.put_gaps();
	
	
	
	
		
	
	//if(paras.print_cbs)
		//cout<<"checking homeless nodes "<<endl;
	
	
	deque<int> homel;
	module_coll.homeless(homel);
	
	int before_procedure= homel.size();
	
	
	if(homel.size()==0)
		return before_procedure;
	
	/*cout<<"homel"<<endl;
	print_id(homel, cout);*/
	
	
	set<int> called;						// modules connected to homeless nodes
	map<int, set<int> > homel_module;		// maps the homeless node with the modules it's connected to
	
	
	
	for(UI i=0; i<homel.size(); i++) {
	
		
		
		set<int> thish;
		for(int j=0; j<vertices[homel[i]]->inlinks->size(); j++) {
		
			int & neigh=vertices[homel[i]]->inlinks->l[j];
			
			for(set<int>:: iterator itk=module_coll.memberships[neigh].begin(); itk!=module_coll.memberships[neigh].end(); itk++) {
			
				called.insert(*itk);
				thish.insert(*itk);
			}
		}
		
		
		for(int j=0; j<vertices[homel[i]]->outlinks->size(); j++) {
		
			int & neigh=vertices[homel[i]]->outlinks->l[j];
			
			for(set<int>:: iterator itk=module_coll.memberships[neigh].begin(); itk!=module_coll.memberships[neigh].end(); itk++) {
			
				called.insert(*itk);
				thish.insert(*itk);
			}
		}

		
		
		
		if(thish.size()>0)
			homel_module[homel[i]]=thish;
	
	
	
	}
	
	
	
	
	map<int, int> module_kin;
	map<int, int> module_ktotin;
	map<int, int> module_ktotout;
	
	for(set<int>:: iterator its=called.begin(); its!=called.end(); its++ ) {
		
		module_kin[*its]=cast_int(kin_m(module_coll.modules[*its]));
		module_ktotin[*its]=cast_int(ktot_m(module_coll.modules[*its]).first);
		module_ktotout[*its]=cast_int(ktot_m(module_coll.modules[*its]).second);
	}
	
	
	
	for(map<int, set<int> > :: iterator itm= homel_module.begin();  itm!=homel_module.end(); itm++) {
	
		
		double cmin=1.1;
		int belongs_to=-1;
		
		//cout<<"homeless node: "<<id_of(itm->first)<<endl;
		
		for(set<int>:: iterator its= itm->second.begin(); its!=itm->second.end(); its++) {
			
			int kin_node_in=cast_int(vertices[itm->first]->kplus_m(module_coll.modules[*its]).first);
			int kin_node_out=cast_int(vertices[itm->first]->kplus_m(module_coll.modules[*its]).second);
			
						
			int kout_g_in= module_ktotin[*its] - module_kin[*its];
			int tmin= cast_int(oneM - module_ktotin[*its]);
			double kinw_in= vertices[itm->first]->kplus_w(module_coll.modules[*its]).first;
			
			int kout_g_out= module_ktotout[*its] - module_kin[*its];
			int tmout= cast_int(oneM - module_ktotout[*its]);
			double kinw_out= vertices[itm->first]->kplus_w(module_coll.modules[*its]).second;

			
			
			
			double rh1= compute_global_fitness_randomized_short(kin_node_in, kout_g_out, tmin, vertices[itm->first]->instub_number, kinw_in);
			double rh2= compute_global_fitness_randomized_short(kin_node_out, kout_g_in, tmout, vertices[itm->first]->outstub_number, kinw_out);
						
			double rh=min(rh1, rh2);
			

			if(rh<cmin) {
				
				cmin=rh;
				belongs_to=*its;
			
			}
			
		}
		
		
		if(belongs_to!=-1) {
			
			if(to_check.find(belongs_to)==to_check.end()) {
				deque<int> void_d;
				to_check[belongs_to]=void_d;
			}
			
			to_check[belongs_to].push_back(itm->first);
		
		}
		
		
		
		//if(paras.print_cbs)
			//cout<<"homeless node: "<<id_of(itm->first)<<" belongs_to "<<belongs_to<<" cmin... "<<cmin<<endl;
			//cherr();
		
		
	}
	
	
	//if(paras.print_cbs)
		//cout<<"homeless node: "<<homel.size()<<" try_to_assign: "<<homel_module.size()<<" modules to check: "<<to_check.size()<<endl;

	
	



	return before_procedure;

}

