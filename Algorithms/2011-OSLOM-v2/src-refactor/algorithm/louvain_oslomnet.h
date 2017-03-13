




class oslom_module {

	
public:
	
	
	oslom_module(int a) {  nc=1; kout=a; ktot=a; };
	~oslom_module(){};
	
	int nc;
	int kout;
	int ktot;
	
};






typedef map<int, pair<int, double> > mapip;
typedef map<int, oslom_module> map_int_om;



void prints(map_int_om & M) {

	
	for(map_int_om :: iterator itm = M.begin(); itm!=M.end(); itm++)
		cout<<"module: "<<itm->first<<"\t\t\t\tnc= "<<itm->second.nc<<"\t ktot= "<<itm->second.ktot<<"\t kout= "<<itm->second.kout<<endl;


}






class oslomnet_louvain : public static_network {
	
public:
	
		
	oslomnet_louvain(): static_network(){};
	~oslomnet_louvain(){};
	
	int collect_raw_groups_once(deque<deque<int> > & );
	
private:

	void weighted_favorite_of(const int & node, int & fi, int & kp, int & kop);
	void unweighted_favorite_of(const int & node, int & fi, int & kp, int & kop);

	void single_pass_unweighted();
	void single_pass_weighted();
	inline void update_modules(const int & i, const int & fi, const int & kp, const int & kpo);
	
		
	void module_initializing();
	void set_partition_collected(deque<deque<int> > & M);

	
	//int check_all();



	map<int, oslom_module> label_module;
	deque<int> vertex_label;
	deque<int> vertex_order;
	deque<bool> vertex_to_check;
	deque<bool> vertex_to_check_next;
	int nodes_changed;
	

};







void oslomnet_louvain::module_initializing() {
	
	for(int i=0; i<dim; i++) {
		
		vertex_label.push_back(i);
		vertex_order.push_back(i);
		vertex_to_check.push_back(true);
		vertex_to_check_next.push_back(false);
		oslom_module newm(vertices[i]->stub_number);
		label_module.insert(make_pair(i, newm));
		
	}
	
}







void oslomnet_louvain::unweighted_favorite_of(const int & node, int & fi, int & kp, int & kop) {
	
	
	
	double min_fitness=10;
	fi=vertex_label[node];
	kp=0;
	kop=0;
	
	
	
	map<int, int>  M;		// M is a map module_label -> kin (internal stubs)
	
	
	for(int j=0; j<vertices[node]->links->size(); j++)
		int_histogram(vertex_label[vertices[node]->links->l[j]], M, vertices[node]->links->w[j].first);


	
	for(map<int, int>:: iterator itM= M.begin(); itM!=M.end(); itM++) {
		
		
		map_int_om :: iterator itOM = label_module.find(itM->first);
		
		double to_fit;
		
		if(itM->first!=vertex_label[node]) {
			
			to_fit = topological_05(itM->second, itOM->second.kout, oneM - itOM->second.ktot, vertices[node]->stub_number);
			
		} else {
			
			kop =  itM->second;
			int kout_prime= itOM->second.kout - vertices[node]->stub_number + 2 *kop;
			to_fit = topological_05(itM->second, kout_prime, oneM - itOM->second.ktot + vertices[node]->stub_number, vertices[node]->stub_number);				
			to_fit*=0.999;		// to break possible ties 
			
		}
		
				
			
		if(to_fit<min_fitness) {
			
			kp=itM->second;
			min_fitness=to_fit;
			fi= itM->first;				
		}
		
	}
	
	
	
	
	
}


void oslomnet_louvain::weighted_favorite_of(const int & node, int & fi, int & kp, int & kop) {
	
	
		
	double min_fitness=10;
	fi=vertex_label[node];
	kp=0;
	kop=0;
	
	
	mapip M;		// M is a map module_label -> kin - win (internal stubs, internal weight)
	
	
	for(int j=0; j<vertices[node]->links->size(); j++)
		int_histogram(vertex_label[vertices[node]->links->l[j]], M, vertices[node]->links->w[j].first,  vertices[node]->links->w[j].second);
	
		
	for(mapip:: iterator itM= M.begin(); itM!=M.end(); itM++) {
		
		
		map_int_om :: iterator itOM = label_module.find(itM->first);
		
		double to_fit;
		
		if(itM->first!=vertex_label[node]) {
			to_fit = topological_05(itM->second.first, itOM->second.kout, oneM - itOM->second.ktot, vertices[node]->stub_number);
			to_fit *= double(dim -itOM->second.nc +1)/ ( min(dim -itOM->second.nc, itOM->second.kout /  itM->second.first +1)+1);
			
	
			if(to_fit>1)	
				to_fit=1;
			
			
			
		} else {
			
			kop =  itM->second.first;
			int kout_prime= itOM->second.kout - vertices[node]->stub_number + 2 *kop;
			to_fit = topological_05(itM->second.first, kout_prime, oneM - itOM->second.ktot + vertices[node]->stub_number, vertices[node]->stub_number);				
			to_fit *= double(dim -itOM->second.nc +2)/ ( min(dim -itOM->second.nc +1, kout_prime /  kop +1) +1);
			
			
			if(to_fit>1)
				to_fit=1;
			to_fit*=0.999;		// to break possible ties 
			
		}
		
		
		
		
		
		double weight_fit= log_together(itM->second.second, itM->second.first);
		double fitness = log_together(-log(to_fit) - log(weight_fit), 2);
		
		

		
		if(fitness<min_fitness) {
			
			kp=itM->second.first;
			min_fitness=fitness;
			fi= itM->first;
		}
		
	}
	
	
	
}




inline void oslomnet_louvain::update_modules(const int & i, const int & fi, const int & kp, const int & kop) {

	
	if(fi!=vertex_label[i]) {
		
		nodes_changed++;
		
		for(int j=0; j<vertices[i]->links->size(); j++)
			vertex_to_check_next[vertices[i]->links->l[j]]=true;
			
		map_int_om :: iterator itm= label_module.find(vertex_label[i]);
		--(itm->second.nc);
		if(itm->second.nc==0)
			label_module.erase(itm);
		else {
			itm->second.kout-= vertices[i]->stub_number - 2 * kop;
			itm->second.ktot-= vertices[i]->stub_number;
		}
		
		itm= label_module.find(fi);
		++(itm->second.nc);
		itm->second.kout+= vertices[i]->stub_number - 2 * kp;
		itm->second.ktot+= vertices[i]->stub_number;
		
		vertex_label[i]=fi;
		

	}
	


}


void oslomnet_louvain::single_pass_unweighted() {
	
	int fi, kp, kop;	// fi is the label node i likes most, kp is the number od internal stubs in module fi, kop is the same for vertex_label[i]  
	
	for(deque<int> :: iterator itd=vertex_order.begin(); itd!=vertex_order.end(); itd++) {

		if(vertex_to_check[*itd]==true) {
			unweighted_favorite_of(*itd, fi, kp, kop);
			update_modules(*itd, fi, kp, kop);
		}
	}

}



void oslomnet_louvain::single_pass_weighted() {
	
	int fi, kp, kop;	// fi is the label node i likes most, kp is the number od internal stubs in module fi, kop is the same for vertex_label[i]  
	
	for(deque<int> :: iterator itd=vertex_order.begin(); itd!=vertex_order.end(); itd++) {

		if(vertex_to_check[*itd]==true) {
			weighted_favorite_of(*itd, fi, kp, kop);
			update_modules(*itd, fi, kp, kop);
		}
	}

}

void oslomnet_louvain::set_partition_collected(deque<deque<int> > & ten2) {

	
	ten2.clear();
	
	deque<deque<int> > M;
	
	
	// take partition from vertex_label  //******************************
	map<int, int> mems;
	for(int i=0; i<dim; i++) {
		
		
		pair<map<int, int>::iterator, bool>  itm_bool= mems.insert(make_pair(vertex_label[i], mems.size()));
		
		if(itm_bool.second==true) {
			deque<int> first;
			M.push_back(first);
		}
		
		
		M[itm_bool.first->second].push_back(i);
	}

	
	
	// check if subgraphs are connected  //******************************
	for(int i=0; i<int(M.size()); i++) {

		deque<deque<int> > link_per_node;
		deque<deque<pair<int, double> > > weights_per_node;
		set_subgraph(M[i], link_per_node, weights_per_node);
		static_network giovanni;
		giovanni.set_graph(link_per_node, weights_per_node, M[i]);
		deque<deque<int> > gM;
		giovanni.set_connected_components(gM);
		
		if(gM.size()==1)
			ten2.push_back(M[i]);
		else {
			for(int j=0; j<int(gM.size()); j++) {
				giovanni.deque_id(gM[j]);
				ten2.push_back(gM[j]);
			}
		}
	
	
	}



}




int oslomnet_louvain::collect_raw_groups_once(deque<deque<int> > & P) {
	
	
	
	module_initializing();
	
	int stopper=0;	
	int previous_nodes_changed=dim;
	int iteration=0;
	
	while (true) {
		
		
		nodes_changed=0;
		
		for(int i=0; i<int(vertex_to_check.size()); i++)
			vertex_to_check_next[i]=false;
		shuffle_s(vertex_order);
		
		if(paras.weighted)
			single_pass_weighted();
		else
			single_pass_unweighted();
	
		if(paras.print_flag_subgraph && iteration%20==0)
			cout<<"iteration: "<<iteration<<" number of modules: "<<label_module.size()<<endl;
		
		++iteration;
		
		/* this conditions means that at least max_iteration_convergence iterations are done and, after that, the number of nodes changed has to decrease (up to a facto 1e-3) */
		if(double(nodes_changed - previous_nodes_changed) > 1e-3 * previous_nodes_changed && iteration>paras.max_iteration_convergence)
			stopper++;

		if(stopper==paras.max_iteration_convergence || nodes_changed==0 || iteration==dim)
			break;
		
		
		vertex_to_check=vertex_to_check_next;
		previous_nodes_changed=nodes_changed;
		
		
	}
	
	set_partition_collected(P);
	
	if(paras.print_flag_subgraph)
		cout<<"collection done "<<endl<<endl<<endl;
	
	
	label_module.clear();
	vertex_label.clear();
	vertex_order.clear();
	vertex_to_check.clear();
	vertex_to_check_next.clear();
	nodes_changed=0;
	
	
	return 0;

}



/*
int oslomnet_louvain::check_all() {
	

	bool print_stuff_oslom_local= false;

	deque<deque<int> > M;
	set_partition_collected(M);
	
	if(print_stuff_oslom_local)
		prints(label_module);
	
	
	
	double one=0;
	double two=0;
	
	for(map_int_om :: iterator itm = label_module.begin(); itm!=label_module.end(); itm++)
		one+=sin(itm->second.nc) + log(itm->second.ktot) + cos(itm->second.kout);
	
	for(int i=0; i<int(M.size()); i++) {
		
		int _ktot= ktot_m(M[i]);
		int _kin= kin_m(M[i]);
		if(print_stuff_oslom_local)
			cout<<"nc: "<<M[i].size()<<" kout: "<<_ktot - _kin<<" ktot: "<<_ktot<<endl;
		two+=sin(M[i].size()) + log( _ktot) + cos(_ktot - _kin);

	
	}
	
	
	cherr(one-two, 1e-6);
	if(print_stuff_oslom_local)
		cout<<"one, two: "<<one<<" "<<two<<endl;
	cout<<"check passed"<<endl;
	return 0;






}
//*/
