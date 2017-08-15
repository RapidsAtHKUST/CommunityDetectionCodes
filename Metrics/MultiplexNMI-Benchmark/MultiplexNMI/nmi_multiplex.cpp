	
#include "standard_package/standard_include.cpp"


int set_partition_from_list(mapii & node_cluster, deque<deque<int> > & one) {

	one.clear();
	
	map<int, int> value_com;
	IT_loop(mapii, itm, node_cluster) {
        value_com.insert(make_pair(itm->second, value_com.size()));
	}
	
    int com=0;
	for (map<int, int>::iterator it=value_com.begin(); it!=value_com.end(); it++)
		it->second=com++;
	
	
	deque <int> first;
	for (int i=0; i<int(value_com.size()); i++)
		one.push_back(first);
		
	
	for (map<int, int>::iterator itm = node_cluster.begin(); itm != node_cluster.end(); itm++) {
		one[value_com[itm->second]].push_back(itm->first);

    }
		
	return 0;

}



void read_level_node_cluster(   string infile, \
                                map<pair<int, int>, int> & tuple_new_id, \
                                int_matrix & partition, SI & levels) {
    
    
    // updates tuple_new_id: {(level, node) : newid}
    // sets partition
    
    partition.clear();
    mapii new_id_cluster;
    
    
    cout<<"reading "<<infile<<endl;
    
    string gins;
    ifstream gin(infile.c_str());
    
    while(getline(gin, gins)) {
        
        DI v;
        cast_string_to_doubles(gins, v);

        if (v.size() == 3) {
            
            pair<int, int> tuple(v[0], v[1]);
            tuple_new_id.insert(make_pair(tuple, tuple_new_id.size()));
            new_id_cluster[tuple_new_id.at(tuple)] = v[2];
            levels.insert(v[0]);
            
        } else if (v.size() > 0){
            cout<<"something wrong with line "<<gins<<endl;
        }
    
    }
    
    set_partition_from_list(new_id_cluster, partition);
    
    /*
    for(map<pair<int, int>, int>::iterator itm = tuple_new_id.begin(); itm != tuple_new_id.end(); itm ++)
        cout<<"("<<itm->first.first<<","<<itm->first.second<<") "<<itm->second<<endl;
    
    cout<<"new_id_cluster"<<endl;
    prints(new_id_cluster);
    printm(partition);
    */
    
}



int main(int argc, char * argv[]) {
	
	if (argc<3) {
		
		cerr<<"Usage: "<<argv[0]<<" file1 file2 "<<endl;
		return -1;
	}
	
	
    int_matrix one;
    int_matrix two;
    
    map<pair<int, int>, int> tuple_new_id;
    
    
    SI levels_one;
    SI levels_two;
    read_level_node_cluster(string(argv[1]), tuple_new_id, one, levels_one);
    read_level_node_cluster(string(argv[2]), tuple_new_id, two, levels_two);
    
    
    if (levels_one != levels_two) {
        cerr<<"=========\nWarning! Layers are not the same in the two files!\n=========\n\n"<<endl;
    }
    
    
    // how many nodes we have in ones and twos
    int ones=0;
	int twos=0;
	RANGE_loop(i, one) ones+=one[i].size();
	RANGE_loop(i, two) twos+=two[i].size();

    
    int common_elements;
	take_common_elements(one, two, common_elements);
    
    cout<<ones<<" tuples (level, node) found in "<<argv[1]<<" #layers: "<<levels_one.size()<<endl;
    cout<<twos<<" tuples (level, node) found in "<<argv[2]<<" #layers: "<<levels_two.size()<<endl;
    cout<<common_elements<<" are in common."<<endl;
    
    
    if(ones != twos) {
        cerr<<"Warning: some nodes are missing in one of the two files"<<endl;
        cerr<<"Only common elements are considered"<<endl;
    } else if(common_elements != ones) {
        cerr<<"Warning! Not all nodes match in the two files"<<endl;
        cerr<<"Only common elements are considered"<<endl;
    } else {
        cerr<<"OK! All tuples match"<<endl;
    }
    
    
	pair<double, double> nmi_vi=mutual_fast(one, two);
	cout<<"Multiplex NMI: "<<nmi_vi.first<<endl;
	
			
	return 0;
}





