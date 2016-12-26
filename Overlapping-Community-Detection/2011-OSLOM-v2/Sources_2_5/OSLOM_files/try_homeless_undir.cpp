
int oslom_net_global::try_to_assign_homeless(module_collection & Mcoll, bool anyway) {
	
	
	Mcoll.put_gaps();
	
	//if(paras.print_cbs)
		//cout<<"checking homeless nodes "<<endl;
	
	deque<int> homel;
	Mcoll.homeless(homel);
	
	int before_procedure = homel.size();
	
	
	
	if(homel.size()==0)
		return before_procedure;
	
	/*cout<<"homel"<<endl;
	print_id(homel, cout);*/
	
	
	set<int> called;						// modules connected to homeless nodes
	map<int, set<int> > homel_module;		// maps the homeless node with the modules it's connected to
	
	
	
	for(UI i=0; i<homel.size(); i++) {
	
		
		
		set<int> thish;
		for(int j=0; j<vertices[homel[i]]->links->size(); j++) {
		
			int & neigh=vertices[homel[i]]->links->l[j];
			
			for(set<int>:: iterator itk=Mcoll.memberships[neigh].begin(); itk!=Mcoll.memberships[neigh].end(); itk++) {
			
				called.insert(*itk);
				thish.insert(*itk);
			}
		}
		
		if(thish.size()>0)
			homel_module[homel[i]]=thish;
	}
	
	
	
	
	map<int, int> module_kin;
	map<int, int> module_ktot;
	
	for(set<int>:: iterator its=called.begin(); its!=called.end(); its++ ) {
		
		module_kin[*its]=cast_int(kin_m(Mcoll.modules[*its]));
		module_ktot[*its]=cast_int(ktot_m(Mcoll.modules[*its]));
	}
	
	
	
	map<int, deque<int> > to_check;			// module - homeless nodes added to that
	for(map<int, set<int> > :: iterator itm= homel_module.begin();  itm!=homel_module.end(); itm++) {
	
		
		double cmin=1.1;
		int belongs_to=-1;
		
		//cout<<"homeless node: "<<id_of(itm->first)<<endl;
		
		for(set<int>:: iterator its= itm->second.begin(); its!=itm->second.end(); its++) {
			
			int kin_node=cast_int(vertices[itm->first]->kplus_m(Mcoll.modules[*its]));
			
			/*cout<<"module: "<<*its<<" kin: "<<module_kin[*its]<<"  ktot: "<<module_ktot[*its]<<" kin h "<<kin_node<<endl;
			print_ri(Mcoll.modules[*its]);*/
			
			int kout_g= module_ktot[*its] - module_kin[*its];
			int tm= oneM - module_ktot[*its];
			//double rh= compute_r_hyper(kin_node, kout_g, tm, vertices[itm->first]->stub_number);
			double kinw= vertices[itm->first]->kplus_w(Mcoll.modules[*its]);
			//double weight_part= log_together(kinw, kin_node);
			
			double rh= compute_global_fitness_randomized_short(kin_node, kout_g, tm, vertices[itm->first]->stub_number, kinw);
			
			//double cs=  1 - pow(1 - rh, dim - Mcoll.modules[*its].size());
			//cout<<"rh: "<<rh<<" ..."<<endl;
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

	
	
	// **** try the groups with the homeless //******************
	bool something=false;
	
	for(map<int, deque<int> > :: iterator itm= to_check.begin();  itm!=to_check.end(); itm++) {
	
		deque<int> union_deque=Mcoll.modules[itm->first];
		
				
		
		for(UI i=0; i<itm->second.size(); i++)
			union_deque.push_back(itm->second[i]);
		
				
		
		if(anyway) {

			something=true;
			Mcoll.insert(union_deque, ran4()+paras.threshold);
		}
		else {
		
			deque<int> grbe;
			double bs=CUP_check(union_deque, grbe);
			
			//cout<<"union_deque after "<<itm->first<<" size: "<<grbe.size()<<endl;
			
			
			if(grbe.size()>1) {
				
				something=true;
				Mcoll.insert(grbe, bs);
				
			}
			
		}
	
	}
	
	
	if(something) {
		Mcoll.compute_inclusions();
	}




	return before_procedure;

}
