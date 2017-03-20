
/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 *	This program is free software; you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by         *
 *  the Free Software Foundation; either version 2 of the License, or            *
 *  (at your option) any later version.                                          *
 *                                                                               *
 *  This program is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
 *  GNU General Public License for more details.                                 *
 *                                                                               *
 *  You should have received a copy of the GNU General Public License            *
 *  along with this program; if not, write to the Free Software                  *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 *  Created by Andrea Lancichinetti on 5/5/09 (email: arg.lanci@gmail.com)       *
 *  Location: ISI foundation, Turin, Italy                                       *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */


//./a.out -f network.dat

#include "visual_net.h"
#include "hier.h"




int get_tp_files(string network_file) {
	
	string s= network_file + "_oslo_files";
	string syst= "ls " + s + " > " + s + "_list";
	cout<<syst<<endl;
	
	char b[1000];
	cast_string_to_char(syst, b);
	int sy= system(b);

	
	string f=s + "_list";
	cast_string_to_char(f, b);
	
	ifstream gin(b);
	int levels=0;
	while(getline(gin, s)) {
	
		if(s.size()>0 && s[0]=='n')
			levels++;
		
	
	}
	
	
	return levels;


}





int main(int argc, char * argv[]) {		
	
	
	srand_file();
	
	if(argc<2) {
	
		cerr<<endl;
		cerr<<"Usage: "<<argv[0]<<" [network_file] [scale_factor]"<<endl<<endl;
		
		cerr<<"The scale_factor is optional. You might want to specify it because the scale sometimes is not perfect. For instance, if you want a bigger visualization, try: "<<endl;
		cout<<argv[0]<<" network_file 2"<<endl<<endl;
		
		
		cerr<<"Output files will be written in the network_file_oslo_files folder"<<endl;
		cerr<<"pos_0 contains the position of the nodes [x, y, id_label]"<<endl;
		cerr<<"the pajek_files are called pajek_files_1.net, pajek_files_2.net and so on, one for each hierarchical level. Enjoy!"<<endl<<endl;
		
		
		return -1;
	}

	
	
	srand_file();
	
	int levels = get_tp_files(string(argv[1]));
	cout<<"levels found: "<<levels<<endl;
	
	
	
	if(levels==0) {
		
		cerr<<"oslo directory not found..."<<endl;
		return -1;
	
	}
	
	
	
	
	
	all_levels(levels, string(argv[1]));

	
	
	static_network luca;
	luca.set_graph(string(argv[1]));
	cout<<"network:: "<<luca.size()<<" nodes"<<endl;		
	
	
	deque<int> node1;
	deque<int> node2;
	deque<double> w12;
	
	get_data_from_file(string(argv[1]), node1, 1);
	get_data_from_file(string(argv[1]), node2, 2);
	get_data_from_file(string(argv[1]), w12, 3);
	
	
	
	

	for(int i=0; i<levels; ++i) {
		
		
		
		
		
		
		deque<deque<int> > M;
		
		char b[1000];
		

		int level= i;
		
		cout<<"writing level: "<<level<<endl;
		
		
		if(level==0)
			sprintf(b, "%s_oslo_files/tp", argv[1]);
		else
			sprintf(b, "%s_oslo_files/tp%d", argv[1], level);

		get_partition_from_file(string(b), M, 1);
		map<int, double> node_x;
		map<int, double> node_y;
		
		
		{
			deque<int> a;
			deque<double> b1;

			sprintf(b, "%s_oslo_files/pos_0", argv[1]);
			get_data_from_file(string(b), a, 3);
			get_data_from_file(string(b), b1, 1);
			
			for(int i=0; i<a.size(); i++)
				node_x[a[i]]=b1[i];
			
			//prints(node_x);
				
		}
		
		{
			deque<int> a;
			deque<double> b1;
			
			sprintf(b, "%s_oslo_files/pos_0", argv[1]);
			get_data_from_file(string(b), a, 3);
			get_data_from_file(string(b), b1, 2);
			
			for(int i=0; i<a.size(); i++)
				node_y[a[i]]=b1[i];
		
		}
		
		
		
		//ofstream ggo("copy_this");
		//ggo<<"unset key\nset xrange[-0.5:1]\nset yrange[-0.5:1.5]"<<endl;
		
		double scale_factor= 1;
		if(argc>2) {
			
			string scale(argv[2]);
			scale_factor = cast_string_to_double(scale);
			cout<<"scale factor selected: "<<scale_factor<<endl;

			//cout<<scale<<endl;
		
		}
		
		
		char output_pajek[1000];
		sprintf(output_pajek, "%s_oslo_files/pajek_file_%d.net", argv[1], level);
		
		
		
		
		
		
		luca.draw_pajek_directed(node_x, node_y, string(output_pajek), M, scale_factor, node1, node2, w12);
		
		
		
		
	
	
	}
	
	
	
	
	//luca.draw_gnuplot(node_x, node_y, ggo, false, M, 0.1);
	
	
	/*
	for(int i=0; i<M.size(); i++) {
		
		sprintf(b, "c_%d", i);
		
		if(i==0)
			ggo<<"p \""<<b<<"\" ps 1.2 pt 4"<<endl;
		else
			ggo<<"rep \""<<b<<"\" ps 1.2 pt 4"<<endl;
		
		
		ofstream go(b);
		for(int j=0; j<M[i].size(); j++)
			go<<node_x[M[i][j]]<<" "<<node_y[M[i][j]]<<" "<<M[i][j]<<endl;
	
	
	}
	*/
		
	
	
	//int sy=system("gnuplot copy_this");
	
	
	
	
	
	
	
	
	return 0;


}



