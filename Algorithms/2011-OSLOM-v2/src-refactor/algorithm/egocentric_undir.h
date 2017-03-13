



class egocentric_net : public oslomnet_louvain {
	
public:
	
	
	egocentric_net(): oslomnet_louvain(){};
	~egocentric_net(){};
	
	int collect_ego_groups_once(deque<deque<int> > & );

	
private:
	
	int add_this_egomodules(int node, module_collection & Mego);
	
	
};



int egocentric_net::collect_ego_groups_once(int_matrix & E) {

	module_collection Mego(dim);
	paras.print_flag_subgraph=false;
	
	for(UI i=0; i<dim; i++) if(vertices[i]->stub_number > 10)
		add_this_egomodules(i, Mego);

	Mego.erase_included();
	Mego.set_partition(E);
	ofstream pout("tpp");
	print_id(E, pout);
	
	
	return 0;

}


int egocentric_net::add_this_egomodules(int node, module_collection & Mego) {

	deque<deque<int> > link_per_node;
	deque<deque<pair<int, double> > > weights_per_node;
	
	DI group;
	for(int j=0; j<vertices[node]->links->size(); j++)
		group.push_back(vertices[node]->links->l[j]);
	
	
	cout<<"........ "<<id_of(node)<<" "<<node<<endl;
	
	set_subgraph(group, link_per_node, weights_per_node);
	oslomnet_louvain ego_subgraph;
	ego_subgraph.set_graph(link_per_node, weights_per_node, group);
	int_matrix A;
	ego_subgraph.collect_raw_groups_once(A);

	if(A.size()>1) {
			
		for(UI i=0; i<A.size(); i++) if(A[i].size()>1) {
			
			ego_subgraph.deque_id(A[i]);
			A[i].push_back(node);
			set<int> sA;
			deque_to_set(A[i], sA);
			set_to_deque(sA, A[i]);
				
			
			cout<<"......... A[i] "<<endl;
			print_id(A[i], cout);
			
			Mego.insert(A[i], 1.);
		}
		
		
		cout<<"*************************************************"<<endl;
		
	
	}


}










