#if !defined(PARTITION_INCLUDED)
#define PARTITION_INCLUDED	
	





int get_partition_from_file(string s, deque<deque<int> > & M) {


	M.clear();
	
	char b[100];
	cast_string_to_char(s, b);
	
	ifstream inb(b);
	
	string st;
	while(getline(inb, st)) {
		
		deque<int> v;
		cast_string_to_doubles(st, v);
		sort(v.begin(), v.end());
		if(!v.empty())
			M.push_back(v);
	
	}

	
	return 0;


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
