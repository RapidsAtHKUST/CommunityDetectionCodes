





class Parameters {
	
	
	public:
	
		Parameters();
		
		~Parameters(){};
		
		int num_nodes;
		double average_k;
		int max_degree;
		double tau;
		double tau2;
		double mixing_parameter;
		int overlapping_nodes;
		int overlap_membership;
		int nmin;
		int nmax;
		bool fixed_range;
		bool excess;
		bool defect;
		bool randomf;
		double clustering_coeff;
		
		
		bool set(string &, string &);
		void set_random();
		bool arrange();
		deque<string> command_flags;
	
		int minC;
		int maxC;
		double mu2;

};

Parameters::Parameters() {
			
			
		num_nodes=unlikely;
		average_k=unlikely;
		max_degree=unlikely;
		
		tau=2;
		tau2=1;
		
		minC=unlikely;
		maxC=unlikely;
		
		mu2=unlikely;
		
		mixing_parameter=unlikely;
		
		overlapping_nodes=0;
		overlap_membership=0;
		
		nmin=unlikely;
		nmax=unlikely;
		
		
		randomf=false;
		fixed_range=false;
		excess=false;
		defect=false;
		
		clustering_coeff=unlikely;
		
		command_flags.push_back("-N");		//0
		command_flags.push_back("-k");		//1
		command_flags.push_back("-maxk");	//2
		command_flags.push_back("-mu1");	//3
		command_flags.push_back("-t1");		//4
		command_flags.push_back("-t2");		//5
		command_flags.push_back("-minc");	//6
		command_flags.push_back("-maxc");	//7
		command_flags.push_back("-on");		//8
		command_flags.push_back("-om");		//9
		command_flags.push_back("-C");		//10
		command_flags.push_back("-minC");		//11
		command_flags.push_back("-maxC");		//12
		command_flags.push_back("-mu2");		//13

			
		
}


void Parameters::set_random() {


	cout<<"this is a random network"<<endl;
	mixing_parameter=0;
	overlapping_nodes=0;
	overlap_membership=0;
	nmax=num_nodes;
	nmin=num_nodes;
	fixed_range=true;
	excess=false;
	defect=false;

}


bool Parameters::arrange() {
		
		
	if(randomf)
		set_random();
	
	
	
	if (num_nodes==unlikely) {
	
		cerr<<"\n***********************\nERROR:\t number of nodes unspecified"<<endl;
		return false;
	
	}
	if (average_k==unlikely) {
	
		cerr<<"\n***********************\nERROR:\t average degree unspecified"<<endl;
		return false;
	
	}
	if (max_degree==unlikely) {
	
		cerr<<"\n***********************\nERROR:\t maximum degree unspecified"<<endl;
		return false;
	
	}
	if (mixing_parameter==unlikely) {
	
		cerr<<"\n***********************\nERROR:\t mixing parameter1 unspecified"<<endl;
		return false;
	
	}
	
	if (mu2==unlikely) {
	
		cerr<<"\n***********************\nERROR:\t mixing parameter2 unspecified"<<endl;
		return false;
	
	}
	
	if (minC==unlikely) {
	
		cerr<<"\n***********************\nERROR:\t minC unspecified"<<endl;
		return false;
	
	}
	
	if (maxC==unlikely) {
	
		cerr<<"\n***********************\nERROR:\t maxC unspecified"<<endl;
		return false;
	
	}
	
	if(overlapping_nodes<0 || overlap_membership<0) {
	
		cerr<<"\n***********************\nERROR:\tsome positive parameters are negative"<<endl;
		
		return false;

	
	}
		
	if (num_nodes<=0 || average_k<=0 || max_degree<=0 || mixing_parameter<0 || (nmax<=0 && nmax!=unlikely) || (nmin<=0 && nmin!=unlikely) ) {
	
		cerr<<"\n***********************\nERROR:\tsome positive parameters are negative"<<endl;
		
		return false;

	
	}
	
	if(mixing_parameter > 1) {
	
		cerr<<"\n***********************\nERROR:\tmixing parameter > 1 (must be between 0 and 1)"<<endl;
		
		return false;

	
	}
	
	if(mu2<0 || mu2>1) {
	
		cerr<<"\n***********************\nERROR:\tmixing parameter 2 must be between 0 and 1"<<endl;
		return false;
	
	}
	
	if(mixing_parameter+mu2>1) {
		
		cerr<<"\n***********************\nERROR:\t mu1 + mu2 must be between 0 and 1"<<endl;
		return false;
		
	}
	
	
	if(minC<=0 || maxC<=0 || maxC < minC) {
	
		cerr<<"\n***********************\nERROR:\tminC or maxC are out of range"<<endl;
		return false;
	}
	

	
			
	if(nmax!= unlikely && nmin!=unlikely)
		fixed_range=true;
	else
		fixed_range=false;
		
		
	if(excess && defect) {
		
		cerr<<"\n***********************\nERROR:\tboth options -inf and -sup cannot be used at the same time"<<endl;
		return false;
	
	}


	
	
	
	cout<<"\n**************************************************************"<<endl;
	cout<<"number of nodes:\t"<<num_nodes<<endl;
	cout<<"average degree:\t"<<average_k<<endl;
	cout<<"maximum degree:\t"<<max_degree<<endl;
	cout<<"exponent for the degree distribution:\t"<<tau<<endl;
	cout<<"exponent for the community size distribution:\t"<<tau2<<endl;
	cout<<"mixing parameter1:\t"<<mixing_parameter<<endl;
	cout<<"mixing parameter2:\t"<<mu2<<endl;
	
	
	cout<<"number of overlapping nodes:\t"<<overlapping_nodes<<endl;
	cout<<"number of memberships of the overlapping nodes:\t"<<overlap_membership<<endl;
	if(clustering_coeff!=unlikely)
		cout<<"Average clustering coefficient: "<<clustering_coeff<<endl;
	
	if (fixed_range) {
		cout<<"community size range set equal to ["<<nmin<<" , "<<nmax<<"]"<<endl;
		
		if (nmin>nmax) {
			cerr<<"\n***********************\nERROR: INVERTED COMMUNITY SIZE BOUNDS"<<endl;
			return false;
		}
		
		if(nmax>num_nodes || maxC > num_nodes) {
			cerr<<"\n***********************\nERROR: maxC BIGGER THAN THE NUMBER OF NODES"<<endl;
			return false;
		}
			
		
	
	}
	
	
		
	cout<<"**************************************************************"<<endl<<endl;
		
	
	
	double martin_mu1= mixing_parameter;
	double martin_mu2= mu2;
	
	mixing_parameter= martin_mu2 / (1. - martin_mu1);
	mu2= martin_mu1;


	return true;


}


bool Parameters::set(string & flag, string & num) {

	// false is something goes wrong
	
	
	
	cout<<"setting... "<<flag<<" "<<num<<endl;
	double err;
	if(!cast_string_to_double(num, err)) {
				
		cerr<<"\n***********************\nERROR while reading parameters"<<endl;
		return false;
			
	}	
		
	if (flag==command_flags[0]) {			
				
		if (fabs(err-int (err))>1e-8) {
				
			cerr<<"\n***********************\nERROR: number of nodes must be an integer"<<endl;
			return false;
			
		}
		
		num_nodes=cast_int(err);
			
			
	}
	else if(flag==command_flags[1]) {
		
		average_k=err;
	
	}
	else if(flag==command_flags[2]) {
		
		max_degree=cast_int(err);
	
	}
	else if(flag==command_flags[3]) {
		
		mixing_parameter=err;
	
	}
	else if(flag==command_flags[4]) {
		
		tau=err;
	
	}
	else if(flag==command_flags[5]) {
		
		tau2=err;
	
	}
	
	else if(flag==command_flags[6]) {
					
		if (fabs(err-int (err))>1e-8) {
						
			cerr<<"\n***********************\nERROR: the minumum community size must be an integer"<<endl;
			return false;
					
		}
					
		nmin=cast_int(err);

	}
	else if(flag==command_flags[11]) {
					
		if (fabs(err-int (err))>1e-8) {
						
			cerr<<"\n***********************\nERROR: the minumum community size minC must be an integer"<<endl;
			return false;
					
		}
					
		minC=cast_int(err);

	}
	else if(flag==command_flags[12]) {
					
		if (fabs(err-int (err))>1e-8) {
						
			cerr<<"\n***********************\nERROR: the maximum community size maxC must be an integer"<<endl;
			return false;
					
		}
					
		maxC=cast_int(err);

	}
	else if(flag==command_flags[13]) {
		
		mu2=err;

	}

	
	else if(flag==command_flags[7]) {
					
		if (fabs(err-int (err))>1e-8) {
						
			cerr<<"\n***********************\nERROR: the maximum community size must be an integer"<<endl;
			return false;
					
		}
					
		nmax=cast_int(err);

	}
	else if(flag==command_flags[8]) {
					
		if (fabs(err-int (err))>1e-8) {
						
			cerr<<"\n***********************\nERROR: the number of overlapping nodes must be an integer"<<endl;
			return false;
					
		}
					
		overlapping_nodes=cast_int(err);

	}
	else if(flag==command_flags[9]) {
					
		if (fabs(err-int (err))>1e-8) {
						
			cerr<<"\n***********************\nERROR: the number of membership of the overlapping nodes must be an integer"<<endl;
			return false;
					
		}
					
		overlap_membership=cast_int(err);

	}
	else if(flag==command_flags[10]) {
					
		clustering_coeff=err;		

	}
	else {
				
		cerr<<"\n***********************\nERROR while reading parameters: "<<flag<<" is an unknown option"<<endl;
		return false;
			
	}
	
	
	
	return true;
	


}




void statement() {
	
	cout<<"\nTo run the program type \n./hbenchmark [FLAG] [P]"<<endl;
	
	cout<<"\n----------------------\n"<<endl;
	cout<<"To set the parameters, type:"<<endl<<endl;
	
	cout<<"-N\t\t[number of nodes]"<<endl;
	cout<<"-k\t\t[average degree]"<<endl;
	cout<<"-maxk\t\t[maximum degree]"<<endl;
	cout<<"-t1\t\t[minus exponent for the degree sequence]"<<endl;
	cout<<"-t2\t\t[minus exponent for the community size distribution]"<<endl;
	cout<<"-minc\t\t[minimum for the micro community sizes]"<<endl;
	cout<<"-maxc\t\t[maximum for the micro community sizes]"<<endl;
	cout<<"-on\t\t[number of overlapping nodes]"<<endl;
	cout<<"-om\t\t[number of memberships of the overlapping nodes]"<<endl;

	cout<<"-minC\t\t[minimum for the macro community size]"<<endl;
	cout<<"-maxC\t\t[maximum for the macro community size]"<<endl;
	
	cout<<"-mu1\t\t[mixing parameter for the macro communities (see Readme file)]"<<endl;
	cout<<"-mu2\t\t[mixing parameter for the micro communities (see Readme file)]"<<endl;
	
	
	
	cout<<"----------------------\n"<<endl;
	cout<<"It is also possible to set the parameters writing flags and relative numbers in a file. To specify the file, use the option:"<<endl;
	cout<<"-f\t[filename]"<<endl;
	
	cout<<"\n-------------------- Examples ---------------------------\n"<<endl;
	cout<<"Example2:"<<endl;
	cout<<"./hbenchmark -f flags.dat"<<endl;
	cout<<"./hbenchmark -N 10000 -k 20 -maxk 50 -mu2 0.3 -minc 20 -maxc 50 -minC 100 -maxC 1000 -mu1 0.1"<<endl;
	
	cout<<"\n-------------------- Other info ---------------------------\n"<<endl;
	cout<<"Read file ReadMe.txt for more info."<<endl<<endl;
	
	
}


bool set_from_file(string & file_name, Parameters & par1) {


	int h= file_name.size();
	char b[h+1];
	cast_string_to_char(file_name, b);
	
	ifstream in(b);
	if (!in.is_open()) {
		cerr<<"File "<<file_name<<" not found. Where is it?"<<endl;
		return false;
	}
	
	
	
	string temp;
	while(in>>temp) {			// input file name

		
		if(temp=="-rand")
			par1.randomf=true;
		
		else if(temp=="-sup")
			par1.excess=true;
		
		else if(temp=="-inf")
			par1.defect=true;
		
		else {
			
				
			string temp2;
			in>>temp2;
			
			
			if(temp2.size()>0) {
			
				
				if(temp=="-f" && temp2!=file_name) {
					if(set_from_file(temp2, par1)==false)
						return false;
				}
				
				if(temp!="-f") {
					if(par1.set(temp, temp2)==false)
						return false;
				}
			
			
			}
		
			else {
			
				cerr<<"\n***********************\nERROR while reading parameters"<<endl;
				return false;
		
			}
		}
		
	}

	
	return true;
	

}



bool set_parameters(int argc, char * argv[], Parameters & par1) {
	
	int argct = 0;
	string temp;
	
	if (argc <= 1) { // if no arguments, return statement about program usage.
		
		statement();
		return false;
	}



		
	while (++argct < argc) {			// input file name

		temp = argv[argct];
		
		
		if(temp=="-rand")
			par1.randomf=true;
		
		
		else if(temp=="-sup")
			par1.excess=true;
		
		else if(temp=="-inf")
			par1.defect=true;
		
		else {
			
			argct++;
				
			string temp2;
				
			if(argct<argc) {
			
				temp2 = argv[argct];
				

				if(temp=="-f") {
					
					if(set_from_file(temp2, par1)==false)
						return false;
					
				}
				

				
				if(temp!="-f") {
					
					
					if(par1.set(temp, temp2)==false)
						return false;
						
						
				}
				
			}
		
			else {
			
				cerr<<"\n***********************\nERROR while reading parameters"<<endl;
				return false;
		
			}
		}
		
	}
	
	
	if(par1.arrange()==false)
		return false;
	
	return true;
}




