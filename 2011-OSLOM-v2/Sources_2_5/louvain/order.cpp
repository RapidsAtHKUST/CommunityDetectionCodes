	






#include "static_network.h"



int main(int argc, char * argv[]) {
	
	
	
	//cout<<"the result is going to be written in blondel.part"<<endl;

	bool value;

	string s;
	string s2;

	if(parse_command_line(value, s, s2, argc, argv)==-1)
		return -1;
	
	static_network luca(s);
	//cout<<"network:: "<<luca.size()<<" nodes and "<<luca.edges()<<" edges;\t average degree = "<<2*luca.edges()/luca.size()<<endl;

	
	luca.draw_consecutive("netconsec.dat", "labelconsec.dat", true);


	int sy= system("./convert -i netconsec.dat -o network.bin -w");
	sy=system("./community network.bin -l -1 -w > network.tree");
	sy = system("./hierarchy network.tree -l 1 > leve1.dat");
	
		

	
		
	deque<deque<int> > one_consec;
	get_partition_from_file_list("leve1.dat", one_consec);
	
	ofstream outt("louvain.part");
	luca.print_id(one_consec, outt);
		
			

	return 0;


}




