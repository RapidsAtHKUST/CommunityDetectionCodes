


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
	
	
	
	{
	
	
		ifstream inb(b1);
		string ins;			
		while(getline(inb, ins)) if(ins.size()>0 && ins[0]!='#') {
			
			deque<string> ds;
			separate_strings(ins, ds);
	
		
			int innum1=cast_int(cast_string_to_double(ds[0]));
			int innum2=cast_int(cast_string_to_double(ds[1]));
					
			double w=1;
			if(ds.size()>2) {
				if(cast_string_to_double(ds[2], w)==false)
					w=1;
			}
															
					
			int new1=newlabels[innum1];
			int new2=newlabels[innum2];
			
			//if(directed || new1<=new2)
			out<<new1+1<<" "<<new2+1<<" "<<w<<endl;

		}
		
		
		
	}
		
	
	
	ofstream lout("labels.dat");
	prints(old_labels, lout);
	
	//system("unixdos network2.dat network.net");
	//system("rm network2.dat");
	
	
	
	return 0;




}


