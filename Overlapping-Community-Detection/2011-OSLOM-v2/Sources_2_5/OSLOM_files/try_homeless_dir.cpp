

int oslom_net_global::try_to_assign_homeless(module_collection & module_coll, bool anyway) {
	
	
	


	map<int, deque<int> > to_check;
	
	
	
	
	int before_procedure = try_to_assign_homeless_help(module_coll, to_check);
	
	bool something=false;
	
	for(map<int, deque<int> > :: iterator itm= to_check.begin();  itm!=to_check.end(); itm++) {
	
		deque<int> union_deque=module_coll.modules[itm->first];
		
		
		for(UI i=0; i<itm->second.size(); i++)
			union_deque.push_back(itm->second[i]);
		
		
		if(anyway) {

			something=true;
			module_coll.insert(union_deque, ran4()+paras.threshold);
		}
		else {
		
			deque<int> grbe;
			double bs=CUP_check(union_deque, grbe);
			
			//cout<<"union_deque after "<<itm->first<<" size: "<<grbe.size()<<endl;
			
			
			if(grbe.size()>1) {
				
				something=true;
				module_coll.insert(grbe, bs);
				
			}
			
		}
	
	}
	
	
	if(something)
		module_coll.compute_inclusions();
	




	return before_procedure;

}



