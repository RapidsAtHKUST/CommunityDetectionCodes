#if !defined(PARTITION_INCLUDED)
#define PARTITION_INCLUDED	
	






int get_partition_from_file_tp_format(string S, deque<deque<int> > & M, deque<double> & bss) {

	bss.clear();
	M.clear();
	
	char b[S.size()+1];
	cast_string_to_char(S, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
	
		deque<string>  vv;
		separate_strings(st, vv);
		if(st.size()>0 && (vv[0]=="#module" || vv[0]=="#group") ) if(cast_string_to_double(vv[5])<1) {
			
			getline(inb, st);
			double bs=cast_string_to_double(vv[5]);
			
			deque<int> v;
			cast_string_to_doubles(st, v);
			sort(v.begin(), v.end());
			if(!v.empty()) {
			
				M.push_back(v);
				bss.push_back(bs);
			}
		}
		
		
	}

	
	return 0;


}




int get_partition_from_file_tp_format_with_homeless(string S, deque<deque<int> > & M, deque<double> & bss) {
	
	bss.clear();
	M.clear();
	
	char b[S.size()+1];
	cast_string_to_char(S, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
		
		deque<string>  vv;
		separate_strings(st, vv);
		if(st.size()>0 && (vv[0]=="#module" || vv[0]=="#group") ) {
			
			getline(inb, st);
			double bs=cast_string_to_double(vv[5]);
			
			deque<int> v;
			cast_string_to_doubles(st, v);
			sort(v.begin(), v.end());
			if(!v.empty()) {
				
				M.push_back(v);
				bss.push_back(bs);
			}
		}
		
		
	}
	
	
	return 0;
	
	
}









int get_partition_from_file_tp_format(string S, map<int, deque<int> > & M) {

	
	// M gives the name of the module and the corresponding deque (all the deques!!!)
	
	M.clear();
	
	char b[S.size()+1];
	cast_string_to_char(S, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
	
		deque<string>  vv;
		separate_strings(st, vv);
		if(st.size()>0 && (vv[0]=="#module") ) {
			
			int name= cast_int(cast_string_to_double(vv[1]));
			
			getline(inb, st);

			deque<int> v;
			cast_string_to_doubles(st, v);
			sort(v.begin(), v.end());
			if(!v.empty())
				M.insert(make_pair(name, v));
			
		}
	}

	
	
	
	return 0;


}



int get_partition_from_file_tp_format(string S, deque<deque<int> > & M, deque<int> & homel) {


	M.clear();
	homel.clear();
	
	char b[S.size()+1];
	cast_string_to_char(S, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
	
		deque<string>  vv;
		separate_strings(st, vv);
		if(st.size()>0 && (vv[0]=="#module" || vv[0]=="#group") ) {
			if(cast_string_to_double(vv[5])<1) {
			
				getline(inb, st);

				deque<int> v;
				cast_string_to_doubles(st, v);
				sort(v.begin(), v.end());
				if(!v.empty())
					M.push_back(v);
		
		
		
			}
			
			else {
			
				getline(inb, st);

				deque<int> v;
				cast_string_to_doubles(st, v);
				if(!v.empty())
					homel.push_back(v[0]);		
		
		
			}
			
			
			
		}
	}

	
	
	sort(homel.begin(), homel.end());
	
	return 0;


}





int get_partition_from_file_tp_format(string S, deque<deque<int> > & M, bool anyway) {


	// if anyway = true il also takes the homeless
	
	M.clear();
	
	char b[S.size()+1];
	cast_string_to_char(S, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
	
		deque<string>  vv;
		separate_strings(st, vv);
		if(st.size()>0 && (vv[0]=="#module" || vv[0]=="#group") ) if(cast_string_to_double(vv[5])<1 || anyway) {
			
			getline(inb, st);

			deque<int> v;
			cast_string_to_doubles(st, v);
			sort(v.begin(), v.end());
			if(!v.empty())
				M.push_back(v);
		}
		
		
	}

	
	return 0;


}


int get_partition_from_file_tp_format(string S, deque<deque<int> > & M) {

	return get_partition_from_file_tp_format(S, M, false);
}


int get_partition_from_file(string s, deque<deque<int> > & M, int min) {

	// only modules with strictly MORE THAN min enter
	
	
	M.clear();
	
	char b[s.size()+1];
	cast_string_to_char(s, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
		
		deque<int> v;
		cast_string_to_doubles(st, v);
		sort(v.begin(), v.end());
		if(!v.empty() && int(v.size())>min)
			M.push_back(v);
	
	}

	
	return 0;


}



void get_partition_from_file(string s, deque<deque<double> > & M) {
    
    ifstream inb(s.c_str());
    M.clear();
    string st;
    while(getline(inb, st)) {
    		
		deque<double> v;
		cast_string_to_doubles(st, v);
        if (v.size()>0)
            M.push_back(v);
    }

}


int get_partition_from_file(string s, deque<deque<int> > & M) {

	return get_partition_from_file(s, M, 0);

}

int get_partition_from_file_list(string s, deque<deque<int> > & ten) {



	ten.clear();
	
	char b[s.size()+1];
	cast_string_to_char(s, b);
	
	ifstream inb(b);
	
	

	
	multimap<int, int> id_value;
	map <int, int> value_com;
	string st;
	while(getline(inb, st)) {
		
		
		deque<int> v;
		cast_string_to_doubles(st, v);
				
		for(int i=1; i<int(v.size()); i++) {
			value_com.insert(make_pair(v[i], value_com.size()));
			id_value.insert(make_pair(v[0], v[i]));
		}
	
	}
	
	
	int com=0;
	for (map<int, int>::iterator it=value_com.begin(); it!=value_com.end(); it++)
		it->second=com++;
	
	
	deque <int> first;
	for (int i=0; i<int(value_com.size()); i++)
		ten.push_back(first);
		
	
	for (multimap<int, int>::iterator itm=id_value.begin(); itm!=id_value.end(); itm++)
		ten[value_com[itm->second]].push_back(itm->first);
	

	
	return 0;




}



void set_partition_from_memberships(DI & node_community, int_matrix & B) {
	
	B.clear();
	int_matrix A;
	
	int hi_number=0;
	RANGE_loop(i, node_community) {
		hi_number=max(hi_number, node_community[i]);
		//cout<<"hi number: "<<hi_number<<endl;
	}
	
	DI f;
	for(int i=0; i<hi_number+1; i++)
		A.push_back(f);
	
	RANGE_loop(i, node_community) if(node_community[i]>=0) {
		//cout<<"mme: "<<*(memberships[i].begin())<<endl;
		A[node_community[i]].push_back(i);
	}
	
	RANGE_loop(i, A) if(A[i].size()>0)
		B.push_back(A[i]);
	
}



int print_tree_format(deque<int_matrix> & ten, ostream & po) {
	
	
	if(ten.size()==0)
		return 0;
	
	deque< map<int, int> > classifications;
	
	
	RANGE_loop(i, ten) {
		
		map<int, int> classification;
		
		cout<<"partition "<<i<<endl;
		printm(ten[i]);
		
		RANGE_loop(j, ten[i]) RANGE_loop(k, ten[i][j]) {
			classification[ten[i][j][k]]=j;
		}
		
		classifications.push_back(classification);
		
	}
	
		
	for(map<int, int>:: iterator itm = classifications[0].begin(); itm!= classifications[0].end(); itm++) {
		
		
		int node = itm->first;
		DI this_node_mem;
		
		
		RANGE_loop(i, classifications) {
			po<<classifications[i][node]<<":";
		}
		
		po<<1<<"\t\""<<node<<"\""<<endl;
		
	
	}
	
	return 0;


}





void take_common_elements(int_matrix & en, int_matrix & ten, int & dim, int min_size=0) {
	
	
	set<int> ens;
	RANGE_loop(i, en) {
		deque_to_set_app(en[i], ens);
	}
	
	set<int> tens;
	RANGE_loop(i, ten) {
		deque_to_set_app(ten[i], tens);
	}
	
	
		
	
	set<int> inte;
	
	for(set<int>::iterator its=ens.begin(); its!=ens.end(); its++) if(tens.find(*its)!=tens.end())
		inte.insert(*its);
	
	
	
	
	int_matrix en2;
	RANGE_loop(i, en) {
		DI new_eni;
		RANGE_loop(j, en[i]) if(inte.find(en[i][j])!=inte.end() )
			new_eni.push_back(en[i][j]);
		
		if(int(new_eni.size())>min_size)
			en2.push_back(new_eni);
	}
	
	
	int_matrix ten2;
	RANGE_loop(i, ten) {
		DI new_eni;
		RANGE_loop(j, ten[i]) if(inte.find(ten[i][j])!=inte.end() )
			new_eni.push_back(ten[i][j]);
		
		if(int(new_eni.size())>min_size)
			ten2.push_back(new_eni);
	}
	
	
	en=en2;
	ten=ten2;
	
	dim=inte.size();
	
	
	
		
	
}






#endif


