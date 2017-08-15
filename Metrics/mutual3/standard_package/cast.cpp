#if !defined(CAST_INCLUDED)
#define CAST_INCLUDED	
	





bool cast_string_to_double (string &b, double &h) {		

// set h= the number written in b[]; 
// return false if there is an error
	
	
	h=0;
	int epresent=0;
		
	for(int i=0; i<b.size(); i++) {
		
		if(b[i]=='e')
			epresent++;
	}
	
	if(epresent>1)
		return false;
	
	
	if(epresent==1) {
	
		string sbefe;
		string safte;
		
		epresent =0;
		
		for(int i=0; i<b.size(); i++) {
			
			
			if (epresent==0 && (b[i]!='e'))
				sbefe.push_back(b[i]);
			
			if (epresent==1 && (b[i]!='e'))
				safte.push_back(b[i]);
				
			if(b[i]=='e') {
				epresent++;
			
			
			}
		
		}
		
		double number_;
		bool bone=cast_string_to_double(sbefe, number_);
		
		double exp_;
		bool btwo=cast_string_to_double(safte, exp_);
		
		if(bone && btwo) {
		
			h=number_ * pow(10, exp_);
			return true;
		
		}
		else
			return false;
		

		
	
	
	}
	
	
	if(b.size()==0)
		return false;
	
	int sign=1;
	
	
	 if (b[0]=='-') {
		
		b[0]='0';
		sign=-1;
	 
	 }
	
	
	
	int digits_before=0;
	for(int i=0; i<b.size(); i++)
		if(b[i]!='.')
			digits_before++;
		else
			break;
	
	
	int j=0;
	
	while (j!=digits_before) {
	
		int number=(int(b[j])-48);
		h+=number*pow(10, digits_before-j-1);
		
		
		if (number<0 || number>9) {
			//cout<<"err "<<number<<endl;
			return false;
		}
		
		j++;
	}
	
	
	j=digits_before+1;
	
	while (j<b.size()) {
		
		int number=(int(b[j])-48);
		h+=number*pow(10, digits_before-j);
		
		if (number<0 || number>9)
			return false;
		
		j++;
	}

		
	h=sign*h;
	
	
	return true;
	
}


double cast_string_to_double(string &b) {


	double h;
	cast_string_to_double(b, h);
	
	return h;

}

int cast_int(double u) {

	int a=int(u);
	if (u - a > 0.5)
		a++;
	
	return a;
		
}


int cast_string_to_char(string file_name, char *b) {

	for (int i=0; i<file_name.size(); i++)
		b[i]=file_name[i];
	b[file_name.size()]='\0';	

	return 0;

}



bool cast_string_to_doubles(string &b, deque<double> & v) {		

	
	
	v.clear();
	string s1;
	
	for(int i=0; i<b.size(); i++) {
		
		
		if(b[i]==' ' || b[i]=='\t' || b[i]=='\n') {
			
			double num;
			if(cast_string_to_double(s1, num))
				v.push_back(num);
			
			s1.clear();
		
		
		} else
			s1.push_back(b[i]);
		
		
		if(i==b.size()-1) {
			
			double num;
			if(cast_string_to_double(s1, num))
				v.push_back(num);
			
			s1.clear();
		
		
		}
			
		
	
	}
	
		
			
	return true;




}


bool cast_string_to_doubles(string &b, deque<int> & v) {		

	
	
	v.clear();
	deque<double> d;
	cast_string_to_doubles(b, d);
	for(int i=0; i<d.size(); i++)
		v.push_back(cast_int(d[i]));
			
	return true;




}







#endif
