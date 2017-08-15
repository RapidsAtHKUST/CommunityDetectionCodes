	


#include "./standard_package/standard_include.cpp"



int main(int argc, char * argv[]) {
	
	
	
	if (argc<3) {
		
		
		
		cerr<<"Usage: ./mutual file1 file2"<<endl;
		return -1;
	
	
	}

	
	
	
	
	int argct = 0;

	string file1(argv[1]);
	string file2(argv[2]);
	
	{
	
		ifstream in1(argv[1]);
		if(in1.is_open()==false) {
			
			cerr<<"file: "<<argv[1]<<" not found"<<endl;
			return -1;
		
		}
	
	
	}
	
	{
	
		ifstream in1(argv[2]);
		if(in1.is_open()==false) {
			
			cerr<<"file: "<<argv[2]<<" not found"<<endl;
			return -1;
		
		}
	
	
	}
	
	
	deque<deque<int> > one;
	deque<deque<int> > two;
	
	
	
		
	get_partition_from_file(file1, one);
	get_partition_from_file(file2, two);
		
	cout<<"mutual3:\t"<<mutual3(one, two)<<endl;
	
	

	
	
			
	return 0;
}



