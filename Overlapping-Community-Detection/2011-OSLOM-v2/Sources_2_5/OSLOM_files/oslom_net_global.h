



class oslom_net_global : public oslomnet_evaluate {
	
	
public:
	
	oslom_net_global(int_matrix & b, deque<deque<pair<int, double> > > & c, deque<int> & d);
	oslom_net_global(string a);
	oslom_net_global(map<int, map<int, pair<int, double> > > & A);
	~oslom_net_global(){};
	
	void hint(module_collection & minimal_modules,  string filename);
	void load(string filename, module_collection & Mall);

	
	void print_modules(bool not_homeless, string tp, module_collection & Mcoll);
	void print_modules(bool not_homeless, ostream & out1, module_collection & Mcoll);



	int try_to_assign_homeless(module_collection & Mcoll, bool anyway);
	void get_covers(string cover_file, int & soft_partitions_written, int gruns);
	void ultimate_cover(string cover_file, int soft_partitions_written, string final_cover_file);
	void print_statistics(ostream & outt, module_collection & Mcoll);


	
private:


	void from_matrix_to_module_collection(int_matrix & good_modules_to_prune, DD & bscores_good, module_collection & minimal_modules);
	void get_cover(module_collection & minimal_modules);
	int try_to_merge_discarded(int_matrix & discarded, int_matrix & good_modules_to_prune, int_matrix & new_discarded, deque<double> & bscores_good);
	void get_single_trial_partition(int_matrix & good_modules_to_prune, deque<double> & bscores_good);
	void single_gather(int_matrix & good_modules_to_prune, deque<double> & bscores_good, int );

	void check_minimality_all(int_matrix & A, DD & bss, module_collection & minimal_modules);	
	void check_minimality_matrix(int_matrix & A, DD & bss, module_collection & minimal_modules, int_matrix & suggestion_matrix, deque<double> & suggestion_bs, int counter);
	bool check_minimality(deque<int> & group, double & bs_group, module_collection & minimal_modules, int_matrix & suggestion_matrix, deque<double> & suggestion_bs);
	
	int check_unions_and_overlap(module_collection & mall, bool only_similar=false);
	void check_existing_unions(module_collection & mall);
	bool fusion_module_its_subs(const DI & A, int_matrix & its_submodules);
	bool fusion_with_empty_A(int_matrix & its_submodules, DI & grc1, double & bs);
	bool check_fusion_with_gather(module_collection & mall);
	int check_intersection(module_collection & Mcoll);
	int check_intersection(deque<int> & to_check, module_collection & Moll);
	int fusion_intersection(set<pair<int, int> > & pairs_to_check, module_collection & Mcoll);
	bool decision_fusion_intersection(int ai1, int ai2, deque<int> & new_insertions, module_collection & Mcoll, double prev_over_percentage);
	
	
};





oslom_net_global::oslom_net_global(map<int, map<int, pair<int, double> > > & A) : oslomnet_evaluate(A) { 
	
	
}




oslom_net_global::oslom_net_global(int_matrix & b, deque<deque<pair<int, double> > > & c, deque<int> & d) : oslomnet_evaluate(b, c, d) { 
	
	
}



oslom_net_global::oslom_net_global(string a) : oslomnet_evaluate(a) { 
	
	
}



int oslom_net_global::try_to_merge_discarded(int_matrix & discarded, int_matrix & good_modules_to_prune, int_matrix & new_discarded, deque<double> & bscores_good) {
		
	new_discarded.clear();	
		

	/* this function is implemented to check if merging discarded modules we can get significant clusters */
	/* discarded is the input, the results are appended, except new_discarded which is cleared */
	
	if(discarded.size()==0)
		return -1;
	
	if(paras.print_flag_subgraph)
		cout<<"checking unions of not significant modules, modules to check: "<<discarded.size()<<endl;

	
	module_collection discarded_modules(dim);
	for(int i=0; i<int(discarded.size()); i++)
		discarded_modules.insert(discarded[i], 1);
		
	map<int, map<int, pair<int, double> > > neigh_weight_s;		// this maps the module id into the neighbor module ids and weights
	set_upper_network(neigh_weight_s, discarded_modules);
	
	if(neigh_weight_s.size()==0)
		return -1;
	
	
	oslom_net_global community_net(neigh_weight_s);		
	//community_net.draw("community_net");	
	
	int_matrix M_raw;		/* M_raw contains the module_id of discarded_modules */
	community_net.collect_raw_groups_once(M_raw);
	
	/*cout<<"community_net partition: "<<endl;
	printm(M_raw);
	cout<<"trying unions of discarded modules... "<<endl;*/
	
	
	for(int i=0; i<int(M_raw.size()); i++) if(M_raw[i].size()>1) {
		
		set<int> l1;
		for(int j=0; j<int(M_raw[i].size()); j++)
			deque_to_set_app(discarded_modules.modules[M_raw[i][j]], l1);		
		
		deque<int> _M_i_;
		set_to_deque(l1, _M_i_);
		//cout<<"merged discarded: "<<M_raw[i].size()<<endl;
		//print_id(_M_i_, cout);
		
		
		deque<int> l;
		double bscore=CUP_check(_M_i_, l);
		
		if(l.size()>0) {
			good_modules_to_prune.push_back(l);
			bscores_good.push_back(bscore);		
		
		} else
			new_discarded.push_back(_M_i_);
	}
	
	



	return 0;

}


void oslom_net_global::get_single_trial_partition(int_matrix & good_modules_to_prune, deque<double> & bscores_good) {
	
	
	/* this function collects significant modules in two steps: 
	   1. using collect_raw_groups_once and cleaning
	   2. putting together discarded modules                   
	   the results are appended in the input data	*/
	
	
	
	int_matrix discarded;
	int_matrix M;
	
	
	
	
	//*****************************************************************************
	collect_raw_groups_once(M);
	
	//cout<<"single_gather"<<endl;
	//print_id(M, cout);
	
	
	//*****************************************************************************
	
	UI total_nodes_tested=0;
	
	for(UI i=0; i<M.size(); i++) {
		
		if(paras.print_flag_subgraph && i%100==0) {			
			cout<<"checked "<<i<<" modules "<<good_modules_to_prune.size()<<" were found significant.  Modules to check: "<<M.size() - i<<". Percentage nodes done: "<<double(total_nodes_tested)/dim<<endl;
		}
		
		/*if(paras.print_flag_subgraph && M[i].size()>1000)
			cout<<"M[i].size(): "<<M[i].size()<<endl;*/

		total_nodes_tested+=M[i].size();

		deque<int> l;
		double bscore;
		
		if(M[i].size()<1000)	
			bscore=group_inflation(M[i], l);
		else		/* if M[i] is big enough the check is faster to save time */
			bscore=CUP_both(M[i], l);
		
		
		if(l.size()>0) {
			good_modules_to_prune.push_back(l);
			bscores_good.push_back(bscore);
		} else
			discarded.push_back(M[i]);
	}
	
	
	if(paras.print_flag_subgraph)
		cout<<"significance check done "<<endl<<endl<<endl;

	
	
	
	//*****************************************************************************
	
	

	int_matrix new_discarded;
	try_to_merge_discarded(discarded, good_modules_to_prune, new_discarded, bscores_good);
	discarded=new_discarded;	
	try_to_merge_discarded(discarded, good_modules_to_prune, new_discarded, bscores_good);
	
	if(paras.print_flag_subgraph)
		cout<<"checking unions of not significant modules done "<<endl<<endl<<endl;

	/* actually here it is also possible to check the new_discarded modules more than twice but I believe this should be enough */
	/* in principle one could do while(try_to_merge_discarded(...)!=-1) */
	

}






void oslom_net_global::single_gather(int_matrix & good_modules_to_prune, deque<double> & bscores_good, int runs=1) {

	
	good_modules_to_prune.clear();
	bscores_good.clear();
	
	for(int i=0; i<runs; i++)
		get_single_trial_partition(good_modules_to_prune, bscores_good);
	
	
}




void oslom_net_global::from_matrix_to_module_collection(int_matrix & good_modules_to_prune, DD & bscores_good, module_collection & minimal_modules) {

	
	
	
	check_minimality_all(good_modules_to_prune, bscores_good, minimal_modules);
	cout<<"***************************************************************************"<<endl;
	cout<<"MINIMALITY CHECK DONE"<<endl;
	
		
	
	
	check_unions_and_overlap(minimal_modules);
	cout<<"***************************************************************************"<<endl;
	cout<<"CHECK UNIONS AND SIMILAR MODULES DONE"<<endl;
	

}


void oslom_net_global::get_cover(module_collection & minimal_modules) {

	/* this function collects the modules using single_gather */
	/* then the good modules are inserted in minimal_modules afetr the check minimality */
	
	paras.print_flag_subgraph=true;
	
	int_matrix good_modules_to_prune;
	deque<double> bscores_good;
	
	
	
	single_gather(good_modules_to_prune, bscores_good);	
	cout<<"***************************************************************************"<<endl;
	cout<<"COLLECTING SIGNIFICANT MODULES DONE"<<endl<<endl;
	
	from_matrix_to_module_collection(good_modules_to_prune, bscores_good, minimal_modules);
	
		


}



void oslom_net_global::check_minimality_all(int_matrix & A, DD & bss, module_collection & minimal_modules) {
	
	
	paras.print_flag_subgraph=false;

	{
		/* simplifying A*/
		module_collection suggestion_mall(dim);
		for(UI i=0; i<A.size(); i++)
			suggestion_mall.insert(A[i], 1);
		
		suggestion_mall.erase_included();
		suggestion_mall.set_partition(A);	
	}
	
	
	
	int counter=0;
	while(A.size()>0) {
		
		
		int_matrix suggestion_matrix;
		deque<double> suggestion_bs;
		

		
		check_minimality_matrix(A, bss, minimal_modules, suggestion_matrix, suggestion_bs, counter);
		
		module_collection suggestion_mall(dim);
		for(UI i=0; i<suggestion_matrix.size(); i++)
			suggestion_mall.insert(suggestion_matrix[i], 1);
		
		suggestion_mall.erase_included();
		suggestion_mall.set_partition(A);
		
	
		

		bss=suggestion_bs;
		++counter;
	
	}
	


}


void oslom_net_global::check_minimality_matrix(int_matrix & A, DD & bss, module_collection & minimal_modules, int_matrix & suggestion_matrix, deque<double> & suggestion_bs, int counter) {

	
	if(A.size()>4)
		cout<<"minimality check: "<<A.size()<<" modules to check, run: "<<counter<<endl;

	if(counter<paras.minimality_stopper) {
		
		for(UI i=0; i<A.size(); i++) {
			
			/*if(i%100==0)		
				cout<<"checked: "<<i<<" modules.  Modules to check... "<<A.size() - i<<endl;*/
			
			
			check_minimality(A[i], bss[i], minimal_modules, suggestion_matrix, suggestion_bs);
		}
	
	
	} else {
	
		for(UI i=0; i<A.size(); i++)
			minimal_modules.insert(A[i], bss[i]);
	}

}



bool oslom_net_global::check_minimality(deque<int> & group, double & bs_group, module_collection & minimal_modules, int_matrix & suggestion_matrix, deque<double> & suggestion_bs) {
	
	
	/*	this function checks the minimality of group 
		minimality means that group doesn't have internal structures up to a factor coverage_percentage_fusion_left
		returns true is group is inserted in minimal_modules	*/
	
	
	int_matrix subM;
	deque<double> bss;
	
	
	{	//******************  module_subgraph stuff   ******************
		
		deque<deque<int> > link_per_node;
		deque<deque<pair<int, double> > > weights_per_node;
		set_subgraph(group, link_per_node, weights_per_node);
		oslom_net_global module_subgraph(link_per_node, weights_per_node, group);
		
		
		DD bscores_good_temp;
		module_subgraph.single_gather(subM, bscores_good_temp);	
		
		for(int i=0; i<int(subM.size()); i++) {
			
			module_subgraph.deque_id(subM[i]);
			
			deque<int> grbe;
			bss.push_back(CUP_check(subM[i], grbe));
			subM[i]=grbe;
			
				
		}			/* so now you know these modules are cleaned (but you are not sure they are minimal) */
		
			
		
	}   //******************  module_subgraph stuff   ******************
	
	

	
	for(UI i=0; i<subM.size(); i++) if (subM[i].size()==group.size()) {
		minimal_modules.insert(group, bs_group);
		return true;
	}
	
	
	set<int> a;
	for(UI i=0; i<subM.size(); i++) for(int j=0; j<int(subM[i].size()); j++)
		a.insert(subM[i][j]);
	
	if(a.size()>paras.coverage_percentage_fusion_left*group.size()) {
		
		/* this means the group cannot be accepted */
		
		
		for(UI i=0; i<subM.size(); i++) if(subM[i].size()>0) {
			suggestion_matrix.push_back(subM[i]);
			suggestion_bs.push_back(bss[i]);
		}
		
		return false;

	} else {
		
		minimal_modules.insert(group, bs_group);
		return true;
		
	}

}


void oslom_net_global::print_modules(bool not_homeless, string tp, module_collection & Mcoll) {
	
	
	
	char b[1000];
	cast_string_to_char(tp, b);
	ofstream out1(b);
	
	print_modules(not_homeless, out1, Mcoll);
	
	
		

}


void oslom_net_global::print_modules(bool not_homeless, ostream & out1, module_collection & Mcoll) {
	
	
	int nmod=0;
	for(map<int, double >::iterator itm = Mcoll.module_bs.begin(); itm!=Mcoll.module_bs.end(); itm++) if(Mcoll.modules[itm->first].size() > 1)		
		nmod++;

	cout<<"******** module_collection ******** "<<nmod<<" modules. writing... "<<endl;
			
	deque<int> netlabs;
	for(int i=0; i<dim; i++)
		netlabs.push_back(id_of(i));
	
	Mcoll.print(out1, netlabs, not_homeless);
	cout<<"DONE   ****************************"<<endl;
	

}





void oslom_net_global::load(string filename, module_collection & Mall) {

	
	// this function is to read a file in the tp-format
	
	
	cout<<"getting partition from tp-file: "<<filename<<endl;
	deque<double> bss;
	deque<deque<int> > A;
	
	get_partition_from_file_tp_format(filename, A, bss);
	translate(A);
	
	cout<<A.size()<<" groups found"<<endl;
	cout<<bss.size()<<" bss found"<<endl;
	
		
	for(UI ii=0; ii<A.size(); ii++) {
		//cout<<"inserting group number "<<ii<<" size: "<<A[ii].size()<<endl;
		Mall.insert(A[ii], bss[ii]);
		
	}
	
	
	
	
}




void oslom_net_global::get_covers(string cover_file, int & soft_partitions_written, int gruns) {
	
	/* this function is to collect different covers of the network
	   they are written in file cover_file (appended)
	   their number is added to soft_partitions_written */
	
	
	char b[1000];
	cast_string_to_char(cover_file, b);
	ofstream out1(b, ios::app);
	

	for(int i=0; i<gruns; i++) {
		
		cout<<"***************************************************************** RUN: #"<<i+1<<endl;

		module_collection Mcoll(dim);
		get_cover(Mcoll);
		
		if(Mcoll.size()>0) {
			print_modules(true, out1, Mcoll);				// not homeless nodes
			soft_partitions_written++;
		}
	}
	
	if(paras.value) {
		
		module_collection Mcoll(dim);
		hint(Mcoll, paras.file2);
		
		if(Mcoll.size()>0) {
			print_modules(true, out1, Mcoll);				// not homeless nodes
			soft_partitions_written++;
		}
	}
	
	
	if(paras.value_load) {
		
		module_collection Mcoll(dim);
		load(paras.file_load, Mcoll);
		
		if(Mcoll.size()>0) {
			print_modules(true, out1, Mcoll);				// not homeless nodes
			soft_partitions_written++;
		}
	}
	

}





void oslom_net_global::ultimate_cover(string cover_file, int soft_partitions_written, string final_cover_file) {


	cout<<"pruning all the modules collected. Partitions found: "<<soft_partitions_written<<endl;
	module_collection Mall(dim);
	load(cover_file, Mall);
	
	if(soft_partitions_written>1)
		check_unions_and_overlap(Mall, true);
	
	cout<<"checking homeless nodes"<<endl;
	if(paras.homeless_anyway==false) {
		try_to_assign_homeless(Mall, false);
	} else {

		deque<int> homel;
		Mall.homeless(homel);
		
		UI before_procedure = homel.size();
		
		while(homel.size()>0) {
			
			cout<<"assigning homeless nodes. Homeless at this point: "<<before_procedure<<endl;
			
			try_to_assign_homeless(Mall, true);
			Mall.homeless(homel);
			if(homel.size()>= before_procedure)
				break;
			before_procedure=homel.size();
			
		}
		
	}
	
	
	Mall.fill_gaps();
	cout<<"writing final solution in file "<<final_cover_file<<endl;
	print_modules(false, final_cover_file, Mall);				// homeless nodes printed


}



void oslom_net_global::hint(module_collection & minimal_modules, string filename) {
	
	
	
	int_matrix good_modules_to_prune;
	deque<double> bscores_good;

	
	cout<<"getting partition from file: "<<filename<<endl;
	int_matrix A;
	
	get_partition_from_file(filename, A);
	
	translate(A);

	cout<<A.size()<<" groups found"<<endl;
		
	for(int ii=0; ii<int(A.size()); ii++) {

		deque<int> group;
		
		cout<<"processing group number "<<ii<<" size: "<<A[ii].size()<<endl;

		double bcu=CUP_both(A[ii], group);
		
		if(group.size()>0) {
			good_modules_to_prune.push_back(group);
			bscores_good.push_back(bcu);
		}
		else
			cout<<"bad group"<<endl;
		
		
	}
	
	
	cout<<"***************************************************************************"<<endl;
	from_matrix_to_module_collection(good_modules_to_prune, bscores_good, minimal_modules);
	
	

}





void oslom_net_global::print_statistics(ostream & outt, module_collection & Mcoll) {
	
	
	int nmod=0;
	UI cov=0;


	for(map<int, double >::iterator itm = Mcoll.module_bs.begin(); itm!=Mcoll.module_bs.end(); itm++) if(Mcoll.modules[itm->first].size() > 1)	{	
		nmod++;
		cov+=Mcoll.modules[itm->first].size();
	}
	
	
	deque<int> homel;
	Mcoll.homeless(homel);
	
	
	
	outt<<"number of modules: "<<nmod<<endl;
	outt<<"number of covered nodes: "<<dim - homel.size()<<" fraction of homeless nodes: "<<double(homel.size())/dim<<endl;
	outt<<"average number of memberships of covered nodes: "<<double(cov)/(dim - homel.size())<<endl;
	
	
	outt<<"average community size: "<<double(cov)/nmod<<endl;
	
	print_degree_of_homeless(homel, outt);
	
}





#include "oslom_net_unions.cpp"
#include "oslom_net_check_overlap.cpp"


