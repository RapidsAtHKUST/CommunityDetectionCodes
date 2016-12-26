	






#include "static_network.h"



int main(int argc, char * argv[]) {
		
	
	int sy=system("cp graph_binary_mac.h graph_binary.h");
	sy=system("make clean");
	sy=system("make");
	sy=system("ls > list_out");
	
	string s;
	ifstream gin("list_out");
	bool compiled=false;
	
	while(gin>>s) {
		
		if(s=="convert")
			compiled=true;
	}
	
	
		
	if(compiled)
		return 0;
	
	cout<<"trying again..."<<endl;
	sy=system("cp graph_binary_linux.h graph_binary.h");
	sy=system("make clean");
	sy=system("make");

	return 0;


}




