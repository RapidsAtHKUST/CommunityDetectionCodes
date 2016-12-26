


int number_together(deque<int> & a) {


	string s;
	
	//cout<<"a"<<endl;
	//prints(a);
	
	char b[100];
	
	for(int i=0; i<a.size(); i++) {
	
		
		
		sprintf(b, "%d", a[i]);
		s+=b;
	
	}
	

	//cout<<s<<endl;
	int l= cast_int(cast_string_to_double(s));
	//cout<<"l::::  "<<l<<endl;
	return l;


}





int get_partition_from_file_list_pajek(string s, deque<deque<int> > & ten, deque<int> & oldlabels) {


	ten.clear();
	
	char b[100];
	cast_string_to_char(s, b);
	
	ifstream inb(b);
	string sst;
	getline(inb, sst);
	
	

	int a=1;
	int bb;
	
	map<int, int> id_value;
	map <int, int> value_com;
	while(inb>>bb) {
		
		value_com.insert(make_pair(bb, value_com.size()));
		id_value.insert(make_pair(a, bb));
		a++;
	
	
	}
	
	
	int com=0;
	for (map<int, int>::iterator it=value_com.begin(); it!=value_com.end(); it++)
		it->second=com++;
	
	
	deque <int> first;
	for (int i=0; i<value_com.size(); i++)
		ten.push_back(first);
		
	
	for (map<int, int>::iterator itm=id_value.begin(); itm!=id_value.end(); itm++)
		ten[value_com[itm->second]].push_back(oldlabels[itm->first-1]);
	

	
	return 0;




}




int set_partition_from_list(deque<int> & mems, deque<deque<int> > & ten) {


	ten.clear();
	
	//cout<<"mems"<<endl;
	//prints(mems);
	
	int a=1;
	
	map<int, int> id_value;
	map <int, int> value_com;
	for(int i=0; i<mems.size(); i++) {
		
		value_com.insert(make_pair(mems[i], value_com.size()));
		id_value.insert(make_pair(a, mems[i]));
		++a;
	
	
	}
	
	
	int com=0;
	for (map<int, int>::iterator it=value_com.begin(); it!=value_com.end(); it++)
		it->second=com++;
	
	
	deque <int> first;
	for (int i=0; i<value_com.size(); i++)
		ten.push_back(first);
		
	
	for (map<int, int>::iterator itm=id_value.begin(); itm!=id_value.end(); itm++)
		ten[value_com[itm->second]].push_back(itm->first);









}



int get_partition_from_file_list_pajek_tree(string s, deque<deque<deque<int> > > & Ten) {

	
	
	
	
	
	
	
	Ten.clear();
	
	char bb[100];
	cast_string_to_char(s, bb);
	
	ifstream inb(bb);
	string sst;
	getline(inb, sst);
	
	
	deque<deque<int> > TREE;
	while(getline(inb, sst)) {
	
		deque<int> sss;
		cast_string_to_doubles(sst, sss);
		TREE.push_back(sss);
	
	}
	
	if(TREE.size()==0)
		return 0;

	
	deque<int> mems(TREE.size());
	
	//int tree_length=TREE[0].size();
	
	for(int ih=0; ih<2; ih++) {		// I want the first two partitions
		
		
		for(int i=0; i<TREE.size(); i++) {
		
		
			
			int finest_level=TREE[i].size()-3;
			int coarse_level= finest_level -1;
			
			
			deque<int> history;
			int this_l=0;
			if(ih==0)
				this_l=coarse_level;
			else
				this_l=finest_level;
				
			
			for(int k=0; k<this_l; k++)
				history.push_back(TREE[i][k]);
			
			//prints(history);
		
			
			//cout<<y<<"-<<<"<<endl;
			//cout<<(TREE[i][tree_length-1]) -1<<" "<<tree_length<<" "<<TREE[i][TREE[i].size()-1]<<" "<<TREE[i].size()<<endl;
			mems[(TREE[i][TREE[i].size()-1]) -1]=number_together(history);
			//cout<<"oks"<<endl;
			
		}
		deque<deque<int> > ten;
		set_partition_from_list(mems, ten);
		Ten.push_back(ten);
		
	
	}
	
	

	cout<<"number of levels: "<<Ten.size()<<endl;
	
	/*for(int i=0; i<Ten.size(); i++) {
	
		
		cout<<"-----------------------"<<endl;
		printm(Ten[i]);
	
	
	}
	*/
	

	
	return 0;




}



int pajek_format(string filename, bool directed) {
	
	// filename is the edge list, directed=true if directed
	
	char b1[1000];
	cast_string_to_char(filename, b1);
	
		
	map<int, int> newlabels;
	deque<int> old_labels;
	
	
	{
	
		int label=0;
		
	
		ifstream inb(b1);
		
		string ins;
		
		
		while(getline(inb, ins)) if(ins.size()>0 && ins[0]!='#') {
			
			
			deque<double> ds;
			cast_string_to_doubles(ins, ds);
			
			if(ds.size()<2 ) {
				cout<<"From file "<<b1<<": string not readable:"<<ins<<endl;				
			}
			
			
			else {
				
				
				int innum1=cast_int(ds[0]);
				int innum2=cast_int(ds[1]);
				
				
				map<int, int>::iterator itf=newlabels.find(innum1);
				if (itf==newlabels.end()) {
					newlabels.insert(make_pair(innum1, label++));
					old_labels.push_back(innum1);
				}
			
				
				
				itf=newlabels.find(innum2);
				if (itf==newlabels.end()) {
					newlabels.insert(make_pair(innum2, label++));
					old_labels.push_back(innum2);
				}
				

					
				
			
			
			}
			
			
			
		}
	
	}
	
	
	int dim=newlabels.size();
	ofstream out("net.paj");
	out<<"*Vertices "<<dim<<endl;

	for(int i=0; i<dim; i++) {
		out<<i+1<<" \""<<old_labels[i]<<"\""<<endl;
	}
	
	if(directed==false)
		out<<"*Edges "<<endl;
	else
		out<<"*Arcs "<<endl;
	
	
	set<pair<int, int> > _pairs_;
	
	{
	
	
		ifstream inb(b1);
		string ins;			
		while(getline(inb, ins)) if(ins.size()>0 && ins[0]!='#') {
			
			deque<string> ds;
			separate_strings(ins, ds);
			
			if(ds.size()>1) {
			
				//prints(ds);
			
				int innum1=cast_int(cast_string_to_double(ds[0]));
				int innum2=cast_int(cast_string_to_double(ds[1]));
						
				double w=1;
				if(ds.size()>2) {
					if(cast_string_to_double(ds[2], w)==false)
						w=1;
				}
																
						
				int new1=newlabels[innum1];
				int new2=newlabels[innum2];
				
				
				//cout<<"***********************************"<<endl;
				//cout<<innum1<<" "<<innum2<<" "<<new1<<" "<<new2<<endl;
				
				if(directed==false) {
					
					int an1= min(new1, new2);
					int an2= max(new1, new2);
					new1=an1;
					new2=an2;
					
				}
				
				pair<int, int> pp(new1, new2);
				if(_pairs_.find(pp)==_pairs_.end() && new1!=new2) {
					
					
					out<<new1+1<<" "<<new2+1<<" "<<w<<endl;
					_pairs_.insert(pp);
				
				}
				
				
			}

		}
		
		
		
	}
		
	
	
	ofstream lout("labels.dat");
	prints(old_labels, lout);
	
	
	
	return 0;




}


