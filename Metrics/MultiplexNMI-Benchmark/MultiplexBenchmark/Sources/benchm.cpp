
#include "standard_bench.cpp"




void sample_graph(ostream & pout, deque<pair<int, int> > & all_edges_deque, double sample_probability, ostream & multiplexout, int layer_index) {

    for(int i=0; i<int(all_edges_deque.size()); i++) {
        if(ran4()<sample_probability) {
            pout<<all_edges_deque[i].first<<" "<<all_edges_deque[i].second<<endl;
            multiplexout<<layer_index<<" "<<all_edges_deque[i].first<<" "<<all_edges_deque[i].second<<" 1"<<endl;
            multiplexout<<layer_index<<" "<<all_edges_deque[i].second<<" "<<all_edges_deque[i].first<<" 1"<<endl;
        }
    }
}


int create_layers(int & layer_index, int num_of_layers, int ori, ostream & comout, int max_assignment_prev, ostream & multiplexout) {
    
    
    // all edges in ori graph
    set<pair<int, int> > all_edges;
    // getting graph
    char buffer[1000];
    sprintf(buffer, "network_%d", ori);
    int n1,n2;
    ifstream gin(buffer);
    while(gin>>n1) {
        gin>>n2;
        all_edges.insert(make_pair(min(n1, n2), max(n1, n2)));
    }
    gin.close();
    cout<<"Read graph #"<<ori<<". Edges: "<<all_edges.size()<<endl;
    deque<pair<int, int> > all_edges_deque;
    for(set<pair<int, int> >::iterator its= all_edges.begin(); its!=all_edges.end(); its++) {
        all_edges_deque.push_back(*its);
    }

    // looping over layers
    
    map<int, int> node_assignment;
    sprintf(buffer, "community_%d", ori);
    ifstream gin2(buffer);
    while(gin2>>n1) {
        gin2>>n2;
        node_assignment[n1]=n2;
    }

    

    int max_assignment=0;
    
    for(int layer=0; layer<num_of_layers; layer++) {

        cout<<"printing layer "<<layer_index<<endl;
        
        sprintf(buffer, "cp community_%d community_layer_%d", ori, layer_index);
        int sy; sy=system(buffer);
        
        for(map<int, int>::iterator itm= node_assignment.begin(); itm!=node_assignment.end(); itm++) {
            comout<<layer_index+1<<" "<<itm->first<<" "<<max_assignment_prev+itm->second<<endl;
            max_assignment=max(max_assignment, max_assignment_prev+itm->second);
        }
        sprintf(buffer, "network_layer_%d", layer_index);
        ofstream multiplex_outfile(buffer);
        sample_graph(multiplex_outfile, all_edges_deque, 1./num_of_layers, multiplexout, layer_index+1);
        ++layer_index;
    }
    
    return max_assignment;
}


int main(int argc, char * argv[]) {
		
	srand_file();
	Parameters p;
	if(set_parameters(argc, argv, p)==false) {
		if (argc>1)
			cerr<<"Please, look at ReadMe.txt..."<<endl;		
		return -1;
	}
	
	erase_file_if_exists("network.dat");
	erase_file_if_exists("community.dat");
	erase_file_if_exists("statistics.dat");

    int sy0= system("rm network_*");
    sy0= system("rm community_*");
    sy0= system("rm network_layer_*");

        
    int num_of_original_graphs=p.num_of_original_graphs;
    int num_of_layers=p.num_of_layers;    
    int layer_index=0;
    
    cout<<"No. of original graphs: "<<num_of_original_graphs<<" No. of layers per graph: "<<num_of_layers<<endl;
    
    int max_assignment=0;
    ofstream comout("level_node_cluster.clu");
    ofstream multiplexout("level_node_node_weight.edges");
    multiplexout<<"*Intra"<<endl;
    
    
    for(int ori=0; ori<num_of_original_graphs; ori++) {
        
        // generating and printing graph
        benchmark(p.excess, p.defect, p.num_nodes, p.average_k, p.max_degree, p.tau, p.tau2, \
                  p.mixing_parameter, p.overlapping_nodes, p.overlap_membership, \
                  p.nmin, p.nmax, p.fixed_range, p.clustering_coeff);	
        
        char buffer[1000];
        sprintf(buffer, "mv network.dat network_%d", ori);
        int sy=system(buffer);
        sprintf(buffer, "mv community.dat community_%d", ori);
        sy=system(buffer);
        system("rm statistics.dat");
        
        // creating layers
        max_assignment= create_layers(layer_index, num_of_layers, ori, comout, max_assignment, multiplexout);
    
    }
    comout.close();
    multiplexout.close();
    
	return 0;
	
}


