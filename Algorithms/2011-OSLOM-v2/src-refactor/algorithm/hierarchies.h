



bool manipulate_string(string s, string netfile, string & outs) {


	outs.clear();
	string find_s("NETx");
	bool netx_found=false;
	
	
	int jj=0;
	string tmp;
		
	for (UI j=0; j<s.size(); j++) {
		
		
		
		tmp.push_back(s[j]);

		
		if(s[j]==find_s[jj]) {
			
			++jj;
			
			if(jj==int(find_s.size())) {
			
				netx_found=true;
				tmp.clear();
				outs+= netfile;
				
			}
		
		
		}
		else {
			jj=0;
			outs+=tmp;
			tmp.clear();
		}	
	
	}

	
	return netx_found;
}




/* this function is to call a different program */
void external_program_to_call(string network_file, oslom_net_global & matteo, string plz_out, int & soft_partitions_written) {

	int sy;
	
	char b[1000];
	cast_string_to_char(plz_out, b);
	ofstream out1(b, ios::app);



	for(UI ei=0; ei<paras.to_run.size(); ei++) {
		
		string output_string;
		
		
		if(manipulate_string(paras.to_run[ei], network_file, output_string)==false) {
			
			cout<<"In string: "<<paras.to_run[ei]<<", keyword NETx was missing. The string cannot be run"<<endl;
		
		} else {
			
			string to_run_p;
			manipulate_string(paras.to_run_part[ei], network_file, to_run_p);
		
			char exec_this[2000];
			cast_string_to_char(output_string, exec_this);
			
			cout<<"running "<<exec_this<<endl;
			sy=system(exec_this);
			
			module_collection Mcoll(matteo.size());
			matteo.hint(Mcoll, to_run_p);
			
			if(Mcoll.size()>0) {
				matteo.print_modules(true, out1, Mcoll);				// not homeless nodes	
				soft_partitions_written++;
			}
			
		}
	
	}



}













void translate_covers(string previous_tp, string new_tp, string short_tp, ostream & stout, int dim) {


	int_matrix M;
	get_partition_from_file_tp_format(previous_tp, M, true);
	char b[1000];
	cast_string_to_char(new_tp, b);
	ofstream outt(b);
	
	int_matrix M2;
	DD bs2;
	
	get_partition_from_file_tp_format_with_homeless(short_tp, M2, bs2);
	
	
	int nmod=0;
	UI cov=0;
	int nhom=0;
	
	
	for(UI i=0; i<M2.size(); i++) {
	
		set<int> S;
		for(UI j=0; j<M2[i].size(); j++)
			deque_to_set_app(M[M2[i][j]], S);
		
		outt<<"#module "<<i<<" size: "<<S.size()<<" bs: "<<bs2[i]<<endl;
		
		for(set<int>:: iterator its=S.begin(); its!=S.end(); its++) {
			outt<<*its<<" ";
		}
		outt<<endl;
		
		
		if(S.size()>1) {
			++nmod;
			cov+=S.size();
		}
		if(S.size()==1)
			++nhom;
		
		
		
	}
	
	
	
	stout<<"number of modules: "<<nmod<<endl;
	stout<<"number of covered nodes: "<<dim - nhom<<" fraction of homeless nodes: "<<double(nhom)/dim<<endl;
	stout<<"average number of memberships of covered nodes: "<<double(cov)/(dim - nhom)<<endl;
	stout<<"average community size: "<<double(cov)/nmod<<endl;

	
	

}





void no_singletons(char * directory_char, oslom_net_global & luca, module_collection & Mcoll) {
	
	
	
	if(paras.homeless_anyway==false) {
		
		deque<set<int> > memberships_;				
		deque<deque<int> > modules_;		
		map<int, double> module_bs_;
		
		memberships_=Mcoll.memberships;
		modules_=Mcoll.modules;
		module_bs_=Mcoll.module_bs;
		
		deque<int> homel;
		Mcoll.homeless(homel);
		
		UI before_procedure = homel.size();
		
		while(homel.size()>0) {
			
			luca.try_to_assign_homeless(Mcoll, true);
			Mcoll.homeless(homel);
			if(homel.size()>= before_procedure)
				break;
			before_procedure=homel.size();
			
		}
		
		char char_to_use[1000];
		sprintf(char_to_use, "%s_oslo_files/tp_without_singletons", directory_char);	
		luca.print_modules(false, string(char_to_use), Mcoll);				// homeless nodes printed
		
		Mcoll.memberships=memberships_;
		Mcoll.modules=modules_;
		Mcoll.module_bs=module_bs_;
		
		
	}
	
}





bool write_tp_of_this_level(int level, oslom_net_global & luca, char * directory_char, int original_dim) {

	
	/*	this function is to compute the hierarchies 
		it returns false when the process can be stopped 
		luca will be set equal to the next network to run	*/
	
	int csy;
	
	char char_to_use[1000];
	sprintf(char_to_use, "%s_oslo_files/partitions_level_%d", directory_char, level);
	string tps(char_to_use);
	if(level==0)
		sprintf(char_to_use, "%s_oslo_files/tp", directory_char);
	else
		sprintf(char_to_use, "%s_oslo_files/short_tp%d", directory_char, level);
	string tp_ultimate(char_to_use);
	
	
	/*********   GETTING COVERS  ***********/
	
	int soft_partitions_written=0;
	if(level==0)
		luca.get_covers(tps, soft_partitions_written, paras.Or);					// here we get covers, which are written in file tps -without homeless-
	else {
		
		paras.homeless_anyway=false;
		paras.value=false;
		paras.value_load=false;
		luca.get_covers(tps, soft_partitions_written, paras.hier_gather_runs);		// here we get covers, which are written in file tps -without homeless-
	}
	
	
	/*********   GETTING COVERS   ***********/
	
	
	
	/*********   extrenal programs ***********/
	
	
	if(level==0) {
		external_program_to_call(paras.file1, luca, tps, soft_partitions_written);
	} else if(paras.to_run.size()>0) {
		
		
		cout<<"copying network file to the main folder"<<endl;
			
		char char_to_copy[1000];
		sprintf(char_to_copy, "cp %s_oslo_files/net%d oslo_network_h", directory_char, level);
		csy=system(char_to_copy);
		external_program_to_call("oslo_network_h", luca, tps, soft_partitions_written);
	}
	
	
	/*********   extrenal programs ***********/
	
	
	luca.ultimate_cover(tps, soft_partitions_written, tp_ultimate);					//here we get the final cover, which is written in file tp_(level) -with homeless-

	
	
	if(level==0) {
		
		sprintf(char_to_use, "cp %s_oslo_files/tp tp", directory_char);
		csy=system(char_to_use);
	}
		
	/* now we get the clusters from the file */
	
	int_matrix A;
	DD bs2;
	get_partition_from_file_tp_format_with_homeless(tp_ultimate, A, bs2);
	module_collection mall(luca.size());
	
	luca.translate(A);
	
	for(UI ii=0; ii<A.size(); ii++)
		mall.insert(A[ii], bs2[ii]);
		
	if(level==0)
		no_singletons(directory_char, luca, mall);
	
	sprintf(char_to_use, "%s_oslo_files/statistics_level_%d.dat", directory_char, level);
	ofstream stout(char_to_use);
	if(level==0)
		luca.print_statistics(stout, mall);

	/* now we construct the community network */
	map<int, map<int, pair<int, double> > > neigh_weight_s;		// this maps the module id into the neighbor module ids and weights
	luca.set_upper_network(neigh_weight_s, mall);
	
	
		
	
	if(level!=0 && int(neigh_weight_s.size())!=luca.size()) {
		// I need to translate the file short_tp%d into tp%d using the content of tp%d-1
		if(level==1)
			sprintf(char_to_use, "%s_oslo_files/tp", directory_char);
		else
			sprintf(char_to_use, "%s_oslo_files/tp%d", directory_char, level -1);
		
		
		string previous_tp(char_to_use);
		sprintf(char_to_use, "%s_oslo_files/tp%d", directory_char, level);
		string new_tp(char_to_use);
		translate_covers(previous_tp, new_tp, tp_ultimate, stout, original_dim);
	}
	
	
	
	
	
	
	double previous_dim=luca.size();
	
	luca.set_graph(neigh_weight_s);
	sprintf(char_to_use, "%s_oslo_files/net%d", directory_char, level+1);
	string net_level(char_to_use);
	if(previous_dim!=luca.size())
		luca.draw(net_level);
	
	
	if(neigh_weight_s.size()==0)
		return false;
	
	if(neigh_weight_s.size()>=(1.-paras.hierarchy_convergence)*previous_dim || neigh_weight_s.size()<=2) {
		
		cout<<"hierarchies done ********* "<<endl;
		return false;
	}

	
	return true;
	
		


}



void oslom_level(oslom_net_global & luca, char * directory_char) {
	
	
	int level=0;
	int original_dim=0;
	
	while(true) {

		cout<<"network:: "<<luca.size()<<" nodes and "<<luca.stubs()<<" stubs;\t average degree = "<<luca.stubs()/luca.size()<<endl;
		if(level==0)
			original_dim=luca.size();


		cout<<"STARTING! HIERARCHICAL LEVEL: "<<level<<endl;
		if(write_tp_of_this_level(level, luca, directory_char, original_dim)==false)
			break;
		
		++level;
		
	}
	


}




