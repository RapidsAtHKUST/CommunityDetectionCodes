

void from_int_matrix_and_deque_to_deque(int_matrix & its_submodules, const DI & A, DI & group) {
	
	// it merges A and its_submodules in group
	
	set<int> all_the_groups;
	for(UI i=0; i<its_submodules.size(); i++) {
		
		
		for(UI j=0; j<its_submodules[i].size(); j++)
			all_the_groups.insert(its_submodules[i][j]);
		
	}
	
	for(UI i=0; i<A.size(); i++)
		all_the_groups.insert(A[i]);
	
	
	set_to_deque(all_the_groups, group);
	


}



bool oslom_net_global::fusion_module_its_subs(const deque<int> & A, deque<deque<int> > & its_submodules) {
	
	
	// A is supposed to be a good cluster
	// return true if A won against its submodules
	// ******************************************
	
	
	if(its_submodules.size()<2)
		return true;
	
	
	
	DI  group;
	from_int_matrix_and_deque_to_deque(its_submodules, A, group);
	
	
	{	//******************  sub_graph_module stuff   ******************
		
		
		
		deque<deque<int> > link_per_node;
		deque<deque<pair<int, double> > > weights_per_node;
		set_subgraph(group, link_per_node, weights_per_node);
		oslom_net_global sub_graph_module(link_per_node, weights_per_node, group);
		
		sub_graph_module.translate(its_submodules);
		
		//------------------------------------ cleaning up submodules --------------------------
		
		
		module_collection sub_mall(sub_graph_module.dim);
		
		for(UI i=0; i<its_submodules.size(); i++)
			sub_mall.insert(its_submodules[i], 1e-3);
		
		
		
		sub_mall.set_partition(its_submodules);
		
		/*
		cout<<"group*************************************************"<<endl;
		print_id(group, cout);
		cout<<"A"<<endl;
		print_id(A, cout);
		cout<<"fusion_module_its_subs"<<endl;
		print_id(its_submodules, cout);
		//*/
		
		//------------------------------------ cleaning up submodules --------------------------
		
		
		set<int> a;
		
		for(UI i=0; i<its_submodules.size(); i++) {
			
			deque<int> grbe;
			sub_graph_module.CUP_check(its_submodules[i], grbe);
			deque_to_set_app(grbe, a);
			//cout<<i<<" cleaned_up: "<<grbe.size()<<" "<<a.size()<<endl;
				
			if(a.size()>paras.coverage_percentage_fusion_or_submodules*A.size())
				return false;
			
		}
		
		
		//sub_graph_module.draw("sub");
		//cherr();
						
		
				
		
		
		
		return true;
		
		
		
	}   //******************  sub_graph_module stuff   ******************
	
	
	
	
}



bool oslom_net_global::fusion_with_empty_A(int_matrix & its_submodules, DI & A, double & bs) {
	
	/*  
		its_submodules are the modules to check. the question is if to take its_submodules or the union of them 
		the function returns true if it's the union, grc1 is the union cleaned and bs the score
	 */
		
	DI group;
	from_int_matrix_and_deque_to_deque(its_submodules, A, group);

	//cout<<"trying a module of "<<group.size()<<" nodes"<<endl;

	bs=CUP_check(group, A);
	
	if(A.size()<=paras.coverage_percentage_fusion_left * group.size()) {
		A.clear();
		bs=1;
		return false;
	}
	
	
	bool fus= fusion_module_its_subs(A, its_submodules);
	

		
	if(fus==false)
		return false;
	else 
		return true;
	
	
	

}



void oslom_net_global::check_existing_unions(module_collection & mall) {
	
	/* this function is to check unions of existing modules*/
	
	/* sorting from the biggest to the smallest module */
	/*cout<<"before check_existing_unions"<<endl;
	print_modules(false, cout, mall);*/
	
	deque<int> sm;
	mall.sort_modules(sm);
	
	/*cout<<"sm"<<endl;
	prints(sm);*/
	
	
	deque<bool> still_good;
	for(UI i=0; i<sm.size(); i++)
		still_good.push_back(true);
	
	set<int> modules_to_erase;
	for(UI i=0; i<sm.size(); i++) {
	
		/* for each module I check if it's better to take it or its submodules */

		
		DI smaller;
		mall.almost_equal(sm[i], smaller);
		int_matrix its_submodules;
		for(UI j=0; j<smaller.size(); j++) if(still_good[smaller[j]])
			its_submodules.push_back(mall.modules[smaller[j]]);
		
		
		/*cout<<"************************** module to check "<<sm[i]<<" size: "<<mall.modules[sm[i]].size()<<endl;
		print_id(mall.modules[sm[i]], cout);
		cout<<"its_submodules"<<endl;
		print_id(its_submodules, cout);*/
		
		if(fusion_module_its_subs(mall.modules[sm[i]], its_submodules)) {
			deque_to_set_app(smaller, modules_to_erase);
			for(UI j=0; j<smaller.size(); j++)
				still_good[smaller[j]]=false;
				
		} else {
			modules_to_erase.insert(sm[i]);
			still_good[sm[i]]=false;
		
		}
				
		
	
	}
	
	for(set<int>:: iterator its= modules_to_erase.begin(); its!=modules_to_erase.end(); its++)
		mall.erase(*its);
	
	
	
	mall.compact();
	/*cout<<"after check_existing_unions --------------------------------------------------------"<<endl;
	print_modules(false, cout, mall);*/
	

}



bool oslom_net_global::check_fusion_with_gather(module_collection & mall) {
	
	
	
	/*	this function is used to check if we would like unions of modules 
		returns true if it merges something	 */
	
	cout<<"check unions of modules using community network"<<endl<<endl;
	paras.print_flag_subgraph=true;
	
	mall.fill_gaps();
	
	
	
	map<int, map<int, pair<int, double> > > neigh_weight_s;		// this maps the module id into the neighbor module ids and weights
	set_upper_network(neigh_weight_s, mall);
	
		
	if(neigh_weight_s.size()==0)
		return false;
	
	bool wgather=true;
	bool real_paras_weighted=paras.weighted;
	
	if(wgather) {
		for (map<int, map<int, pair<int, double> > > :: iterator itm= neigh_weight_s.begin(); itm!=neigh_weight_s.end(); itm++) {
			for(map<int, pair<int, double> >  :: iterator itm2= itm->second.begin(); itm2!=itm->second.end(); itm2++) {
				itm2->second.first=1;
			}
		}
		paras.weighted=true;
	}
	
	
	oslom_net_global community_net(neigh_weight_s);
	
	
	
	int_matrix M_raw;		/* M_raw contains the module_ids */
	community_net.collect_raw_groups_once(M_raw);

	paras.weighted=real_paras_weighted;

	bool something=false;
	int_matrix module_to_insert;
	DD bs_to_insert;
	
	int fused_modules=0;
	
	cout<<"possible fusions to check: "<<M_raw.size()<<endl;
	
	
	for(UI i=0; i<M_raw.size(); i++) if(M_raw[i].size()>1) {
		
		int_matrix ten;
		for(UI j=0; j<M_raw[i].size(); j++)
			ten.push_back(mall.modules[M_raw[i][j]]);		
		
		//cout<<"trying fusion # "<<i<<" "<<ten.size()<<" modules to merge"<<endl;
		
		DI grc1;
		double bs;
		if(fusion_with_empty_A(ten, grc1, bs)) {
			
			something=true;
			
			module_to_insert.push_back(grc1);
			++fused_modules;
			bs_to_insert.push_back(bs);
		}
		
		
		if(i%100==0)
			cout<<"checked "<<i<<" unions. Fused: "<<fused_modules<<endl;
		
		
	}
	
	for(UI i=0; i<module_to_insert.size(); i++)
		mall.insert(module_to_insert[i], bs_to_insert[i]);
	
	mall.compute_inclusions();
	return something;

}




int oslom_net_global::check_unions_and_overlap(module_collection & mall, bool only_similar) {
	
	
	mall.put_gaps();
	
	if(mall.effective_groups()==0)
		return 0;

	
	cout<<"checking similar modules"<<endl<<endl;
	check_existing_unions(mall);
	
	if(only_similar==false) {
	
		if(check_fusion_with_gather(mall))
			check_fusion_with_gather(mall);
	}
	
	cout<<"checking highly intersecting modules"<<endl<<endl;
	check_intersection(mall);
	mall.compute_inclusions();
	
	
	
	return 0;
	
}






