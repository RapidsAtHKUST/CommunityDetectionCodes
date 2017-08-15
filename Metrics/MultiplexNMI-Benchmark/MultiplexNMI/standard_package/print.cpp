#if !defined(PRINT_INCLUDED)
#define PRINT_INCLUDED	
	


void cherr() {
	
	cerr<<"the check failed"<<endl;
	int e;
	cin>>e;
	
}


void cherr(double a) {
	
	cerr<<"the check failed because of "<<a<<endl;
	int e;
	cin>>e;
	
}


void cherr(double a, double ee) {
	
	if(fabs(a)>ee) {
	
		cerr<<"the check failed because of "<<a<<endl;
		int e;
		cin>>e;
	}
	
	
}


template <typename uno, typename due>
void prints(pair <uno, due> &sq,  ostream &out) {

	out<<sq.first<<" "<<sq.second<<endl;
	
}


template <typename uno, typename due>
void prints(pair <uno, due> &sq) {

	cout<<sq.first<<"\t"<<sq.second<<endl;
	
}


template <typename uno, typename due>
void prints(map <uno, due> &sq,  ostream &out) {

	typename map <uno, due>::iterator it = sq.begin(); 
	while(it != sq.end()) { 
		out<<it->first<<"\t"<<it->second<<endl;
		it++; 
	} 

	out<<endl;
	
}

template <typename uno, typename due>
void prints(multimap <uno, due> &sq,  ostream &out) {

	typename map <uno, due>::iterator it = sq.begin(); 
	while(it != sq.end()) { 
		out<<it->first<<"\t"<<it->second<<endl;
		it++; 
	} 

	out<<endl;
	
}



template <typename Seq>
void prints(Seq &sq, ostream &out) {

	typename Seq::iterator it = sq.begin(); 
	while(it != sq.end())
		out<<*(it++)<<" ";
		

	out<<endl;
	
}

template <typename type_>
void prints(type_ *a, int b) {
	
	for (int i=0; i<b; i++)
		cout<<a[i]<<" ";
	cout<<endl;


}


 
	


template <typename uno, typename due>
void prints(map <uno, due> &sq) {

	typename map <uno, due>::iterator it = sq.begin(); 
	while(it != sq.end()) { 
		cout<<it->first<<"\t"<<it->second<<endl;
		it++; 
	} 

	cout<<endl;
	
}

template <typename uno, typename due>
void prints(multimap <uno, due> &sq) {

	typename map <uno, due>::iterator it = sq.begin(); 
	while(it != sq.end()) { 
		cout<<it->first<<"\t"<<it->second<<endl;
		it++; 
	} 

	cout<<endl;
	
}


template <typename uno, typename due>
void prints(deque<uno> & a, deque<due> &b) {

	for(int i=0; i<a.size(); i++)
		cout<<a[i]<<" "<<b[i]<<endl;
	
}

template <typename uno, typename due>
void prints(deque<uno> & a, deque<due> &b, ostream &out) {

	for(int i=0; i<a.size(); i++)
		out<<a[i]<<" "<<b[i]<<endl;
	
}


template <typename Seq>
void prints(Seq &sq) {

	typename Seq::iterator it = sq.begin(); 
	while(it != sq.end())
		cout<<*(it++)<<" ";
		

	cout<<endl;
	
}


template <typename type>
void prints(const deque<type> & sq) {
	
	for(UI i=0; i<sq.size(); i++)
		cout<<sq[i]<<"\t";
	cout<<endl;
	
		
}

template <typename type>
void prints(const vector<type> & sq) {
	
	for(int i=0; i<sq.size(); i++)
		cout<<sq[i]<<"\t";
	cout<<endl;
	
		
}




template <typename type>
void printm(deque<type> & M, ostream & pout) { 
	for(UI i=0; i<M.size(); i++)
		prints(M[i], pout);
} 

template <typename type>
void printm(deque<type> & M) { 
	printm(M, cout);
} 


template <typename type>
void printm(vector<type> & M) { 
	
	
	for(int i=0; i<M.size(); i++)
		prints(M[i]);
	cout<<endl;
} 





template <typename type>
void get_data_from_file(string s, deque<type> & a1, int col) {

	
	// default will be col=1
	
	
	char b[s.size()+1];
	cast_string_to_char(s, b);
	ifstream lin(b);
	
	
	a1.clear();
	col--;
	
	string sas;
	while(getline(lin, sas)) {
		
		deque<double> s;
		cast_string_to_doubles(sas, s);
		if(int(s.size())>col) {
			a1.push_back(s[col]);
		}
	
	}
	
	

}



template <typename type>
void get_data_from_file(string s, deque<type> & a1) {

	get_data_from_file(s, a1, 1);	

}



template <typename type>
void get_data_from_file(string s, deque<type> & a1, deque<type> & a2, int col1, int col2) {

	
	// default will be col1=1, col2=2
	
	
	char b[s.size()+1];
	cast_string_to_char(s, b);
	ifstream lin(b);
	
	
	a1.clear();
	a2.clear();
	col1--;
	col2--;
	
	string sas;
	while(getline(lin, sas)) {
		
		deque<double> s;
		cast_string_to_doubles(sas, s);
		if(s.size()>col2) {
			
			a1.push_back(s[col1]);
			a2.push_back(s[col2]);
		
		}
	
	}
	
	
	
}



template <typename type>
void get_data_from_file(string s, deque<type> & a1, deque<type> & a2) {

	
	get_data_from_file(s, a1, a2, 1, 2);
	

}



void get_data_from_file_string(string s, deque<string> & a1, int col) {

	
	// default will be col=1
	
	
	char b[s.size()+1];
	cast_string_to_char(s, b);
	ifstream lin(b);
	
	
	a1.clear();
	col--;
	
	string sas;
	while(getline(lin, sas)) {
		
		deque<string> v;
		separate_strings(sas,  v);
		 
		 //prints(v);
		 
		if(int(s.size())>col) {
			a1.push_back(v[col]);
		}
	
	}
	
	

}


bool check_if_file_exists(string netfile) {			/* check if file_name exists */
	
	char b[netfile.size()+1];
	cast_string_to_char(netfile, b);
	ifstream inb(b);
	if(inb.is_open()==false) {
		
		cout<<"File "<<netfile<<" not found"<<endl;
		return false;
		
	}	
	
	return true;
	
}



#endif
