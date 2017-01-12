#if !defined(PARTITION_INCLUDED)
#define PARTITION_INCLUDED	
	










int get_partition_from_file_tp_format(string S, map<int, deque<int> > & M) {

	
	// M gives the name of the module and the corresponding deque (all the deques!!!)
	
	M.clear();
	
	char b[100];
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
	
	char b[100];
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
	
	char b[100];
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


	M.clear();
	
	char b[100];
	cast_string_to_char(s, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
		
		deque<int> v;
		cast_string_to_doubles(st, v);
		sort(v.begin(), v.end());
		if(!v.empty() && v.size()>min)
			M.push_back(v);
	
	}

	
	return 0;


}



int get_partition_from_file(string s, deque<deque<int> > & M) {

	return get_partition_from_file(s, M, 0);

}

int get_partition_from_file_list(string s, deque<deque<int> > & ten) {



	ten.clear();
	
	char b[100];
	cast_string_to_char(s, b);
	
	ifstream inb(b);
	
	

	
	multimap<int, int> id_value;
	map <int, int> value_com;
	string st;
	while(getline(inb, st)) {
		
		
		deque<int> v;
		cast_string_to_doubles(st, v);
				
		for(int i=1; i<v.size(); i++) {
			value_com.insert(make_pair(v[i], value_com.size()));
			id_value.insert(make_pair(v[0], v[i]));
		}
	
	}
	
	
	int com=0;
	for (map<int, int>::iterator it=value_com.begin(); it!=value_com.end(); it++)
		it->second=com++;
	
	
	deque <int> first;
	for (int i=0; i<value_com.size(); i++)
		ten.push_back(first);
		
	
	for (multimap<int, int>::iterator itm=id_value.begin(); itm!=id_value.end(); itm++)
		ten[value_com[itm->second]].push_back(itm->first);
	

	
	return 0;




}





#endif


