


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
 *  Created by Andrea Lancichinetti on 7/01/09 (email: arg.lanci@gmail.com)      *
 *	Modified on 9/2/11                                                           *
 *	Collaborators: Santo Fortunato												 *
 *  Location: ISI foundation, Turin, Italy                                       *
 *	Project: Benchmarking community detection programs                           *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */



#include "binary_benchm.cpp"



int print_network(deque<set<int> > & E, const deque<deque<int> > & member_list, const deque<deque<int> > & member_matrix, deque<int> & num_seq) {

	
	int edges=0;

		
	int num_nodes=member_list.size();
	
	deque<double> double_mixing;
	for (int i=0; i<E.size(); i++) {
		
		double one_minus_mu = double(internal_kin(E, member_list, i))/E[i].size();
		
		double_mixing.push_back(1.- one_minus_mu);
				
		edges+=E[i].size();
		
	}
	
	
	//cout<<"\n----------------------------------------------------------"<<endl;
	//cout<<endl;
	
	
	
	double density=0; 
	double sparsity=0;
	
	for (int i=0; i<member_matrix.size(); i++) {

		double media_int=0;
		double media_est=0;
		
		for (int j=0; j<member_matrix[i].size(); j++) {
			
			
			double kinj = double(internal_kin_only_one(E[member_matrix[i][j]], member_matrix[i]));
			media_int+= kinj;
			media_est+=E[member_matrix[i][j]].size() - double(internal_kin_only_one(E[member_matrix[i][j]], member_matrix[i]));
					
		}
		
		double pair_num=(member_matrix[i].size()*(member_matrix[i].size()-1));
		double pair_num_e=((num_nodes-member_matrix[i].size())*(member_matrix[i].size()));
		
		if(pair_num!=0)
			density+=media_int/pair_num;
		if(pair_num_e!=0)
			sparsity+=media_est/pair_num_e;
		
		
	
	}
	
	density=density/member_matrix.size();
	sparsity=sparsity/member_matrix.size();
	
	
	


	/*ofstream out1("network.dat");
	for (int u=0; u<E.size(); u++) {

		set<int>::iterator itb=E[u].begin();
	
		while (itb!=E[u].end())
			out1<<u+1<<"\t"<<*(itb++)+1<<endl;
		
		

	}*/
		

	/*
	ofstream out2("community_first_level.dat", ios::app);

	for (int i=0; i<member_list.size(); i++) {
		
		out2<<i+1<<"\t";
		for (int j=0; j<member_list[i].size(); j++)
			out2<<member_list[i][j]+1<<" ";
		out2<<endl;
	
	}

	cout<<"\n\n---------------------------------------------------------------------------"<<endl;
	*/
	
	cout<<"sub-network of "<<num_nodes<<" vertices and "<<edges/2<<" edges"<<";\t average degree = "<<double(edges)/num_nodes<<endl;
	//cout<<"\naverage mixing parameter: "<<average_func(double_mixing)<<" +/- "<<sqrt(variance_func(double_mixing))<<endl;
	cout<<"p_in: "<<density<<"\tp_out: "<<sparsity<<endl;

	
	
	/*ofstream statout("statistics.dat");
	
	deque<int> degree_seq;
	for (int i=0; i<E.size(); i++)
		degree_seq.push_back(E[i].size());
	
	statout<<"degree distribution (probability density function of the degree in logarithmic bins) "<<endl;
	log_histogram(degree_seq, statout, 10);
	statout<<"\ndegree distribution (degree-occurrences) "<<endl;
	int_histogram(degree_seq, statout);
	statout<<endl<<"--------------------------------------"<<endl;

		
	statout<<"community distribution (size-occurrences)"<<endl;
	int_histogram(num_seq, statout);
	statout<<endl<<"--------------------------------------"<<endl;

	statout<<"mixing parameter"<<endl;
	not_norm_histogram(double_mixing, statout, 20, 0, 0);
	statout<<endl<<"--------------------------------------"<<endl;
	
	
	*/


	cout<<endl<<endl;

	return 0;

}


	



int benchmark(bool excess, bool defect, int num_nodes, double  average_k, int  max_degree, double  tau, double  tau2, 
	double  mixing_parameter, int  overlapping_nodes, int  overlap_membership, int  nmin, int  nmax, bool  fixed_range, double ca, deque<set<int> > & E, deque<deque<int> > & member_list, 
	deque<deque<int> > & link_list, deque<int> & external_stubs, double mu2) {	

	
	
	
	
	double dmin=solve_dmin(max_degree, average_k, -tau);
	if (dmin==-1)
		return -1;
	
	int min_degree=int(dmin);
	
	
	double media1=integer_average(max_degree, min_degree, tau);
	double media2=integer_average(max_degree, min_degree+1, tau);
	
	if (fabs(media1-average_k)>fabs(media2-average_k))
		min_degree++;
	
	
	
	
	
	
		
	// range for the community sizes
	if (!fixed_range) {
	
		nmax=max_degree;
		nmin=max(int(min_degree), 3);
		cout<<"-----------------------------------------------------------"<<endl;
		cout<<"community size range automatically set equal to ["<<nmin<<" , "<<nmax<<"]"<<endl;
		

	}
	
	
	
	//----------------------------------------------------------------------------------------------------
	
	
	deque <int> degree_seq ;		//  degree sequence of the nodes
	deque <double> cumulative;
	powerlaw(max_degree, min_degree, tau, cumulative);
	
	for (int i=0; i<num_nodes; i++) {
		
		int nn=lower_bound(cumulative.begin(), cumulative.end(), ran4())-cumulative.begin()+min_degree;
		degree_seq.push_back(nn);
	
	}
	
	
	
	sort(degree_seq.begin(), degree_seq.end());
		
	if(deque_int_sum(degree_seq) % 2!=0)
		degree_seq[max_element(degree_seq.begin(), degree_seq.end()) - degree_seq.begin()]--;
	
	
	
	
	
	for(int i=0; i<degree_seq.size(); i++) {
	
		
		double exter=mu2*degree_seq[i];		
		int int_exter=int(exter);
		if (ran4()<(exter-int_exter))
			int_exter++;
		
		
		//cout<<degree_seq[i]<<" "<<int_exter<<" "<<mu2<<" "<<mu2*degree_seq[i]<<endl;
		degree_seq[i]-=int_exter;
		external_stubs.push_back(int_exter);
		
	
	}
	
	
	
	
	
	
	
	
	
	deque<deque<int> >  member_matrix;
	deque<int> num_seq;
	deque<int> internal_degree_seq;
	
	// ********************************			internal_degree and membership			***************************************************

	
	

	if(internal_degree_and_membership(mixing_parameter, overlapping_nodes, overlap_membership, num_nodes, member_matrix, excess, defect, degree_seq, num_seq, internal_degree_seq, fixed_range, nmin, nmax, tau2)==-1)
		return -1;
	
	
	
	E.clear();						// E is the adjacency matrix written in form of list of edges
	member_list.clear();			// row i cointains the memberships of node i
	link_list.clear();				// row i cointains degree of the node i respect to member_list[i][j]; there is one more number that is the external degree

	
	
	cout<<"building communities... "<<endl;
	if(build_subgraphs(E, member_matrix, member_list, link_list, internal_degree_seq, degree_seq, excess, defect)==-1)
		return -1;	
	




	cout<<"connecting communities... "<<endl;
	connect_all_the_parts(E, member_list, link_list);
	


	if(erase_links(E, member_list, excess, defect, mixing_parameter)==-1)
		return -1;

	
	
	/*
	if(ca!=unlikely) {
		cout<<"trying to approach an average clustering coefficient ... "<<ca<<endl;
		cclu(E, member_list, member_matrix, ca);
	}
	*/
	
	
	
	print_network(E, member_list, member_matrix, num_seq);

	
	
	
		
	return member_matrix.size();
	
}



void erase_file_if_exists(string s) {

	char b[100];
	cast_string_to_char(s, b);
	
	
	ifstream in1(b);
	
	if(in1.is_open()) {
		
		char rmb[120];
		sprintf(rmb, "rm %s", b);

		int erase= system(rmb);
	}


}



int choose_macro_sizes(int N, deque<int> & msizes, int minC, int maxC, double tau2) {

	
	
	
	
	deque<double> cumulative;
	powerlaw(maxC, minC, tau2, cumulative);
		

	
	int _num_=0;
	
	while (true) {
		
		int nn=lower_bound(cumulative.begin(), cumulative.end(), ran4())-cumulative.begin()+minC;
		
		if (nn+_num_<=N) {
			
			msizes.push_back(nn);				
			_num_+=nn;
		
		}
		else
			break;
	}
	
	msizes[min_element(msizes.begin(), msizes.end()) - msizes.begin()]+=N - _num_;
	
	return 1;


}



int main(int argc, char * argv[]) {
	
		
	erase_file_if_exists("community_first_level.dat");
	erase_file_if_exists("community_second_level.dat");
	erase_file_if_exists("network.dat");
	

	
	srand_file();
	Parameters p;
	if(set_parameters(argc, argv, p)==false) {
		
		if (argc>1)
			cerr<<"Please, look at ReadMe.txt..."<<endl;
		
		return -1;
	
	}
	
	
		
	

	
	
	deque<set<int> > E_global;
	deque<deque<int> > link_list_global;
	deque<deque<int> > member_list_global;
	deque<deque<int> > member_list_global_second_level;
	deque<int> external_stubs;

	int global_nodes=0;
	int global_coms=0;
	
	
	
	int current_graph=0;
	
	
	deque<int> msizes;
	choose_macro_sizes(p.num_nodes, msizes, p.minC, p.maxC, p.tau2);

	
	
	
	
	for(int k=0; k<msizes.size(); k++)  {
		
		current_graph++;
		cout<<"building subgraph number "<<current_graph<<endl<<endl<<endl;
		
		char b[1000];
		
		
			
		
		deque<set<int> > E;
		deque<deque<int> > link_list;
		deque<deque<int> > member_list;
		int newcoms= benchmark(p.excess, p.defect, msizes[k], p.average_k, p.max_degree, p.tau, p.tau2, p.mixing_parameter, p.overlapping_nodes, p.overlap_membership, 
		p.nmin, p.nmax, p.fixed_range, p.clustering_coeff, E, member_list, link_list, external_stubs, p.mu2);	
		
		if(newcoms<0) {
			
			cerr<<"EXIT... "<<endl;
			return -1;
		
		}
		
		for(int i=0; i<E.size(); i++) {
			
			set<int> as;
			for(set<int>:: iterator its=E[i].begin(); its!=E[i].end(); its++)
				as.insert(*its+global_nodes);
			
			E_global.push_back(as);
			
			deque<int> ass;
			for(deque<int>:: iterator its=member_list[i].begin(); its!=member_list[i].end(); its++)
				ass.push_back(*its+global_coms);
			

			member_list_global.push_back(ass);
			link_list_global.push_back(link_list[i]);
			deque<int> mlgsl;
			mlgsl.push_back(current_graph);
			member_list_global_second_level.push_back(mlgsl);
			
		
		}
		
		global_nodes+=E.size();
		global_coms+=newcoms;
		
		cout<<"*************************************************"<<endl;
		cout<<"*************************************************"<<endl;

		
	}
	
	
	// ******************************************* adding external stubs ****************************
	
	
	for(int i=0; i<link_list_global.size(); i++) {
		
		//cout<<"-> "<<degree_i<<endl;
		link_list_global[i].push_back(external_stubs[i]);
	
	}
	
	
	
	cout<<"connecting macro communities"<<endl;
	connect_all_the_parts(E_global, member_list_global_second_level, link_list_global);
	

	
	
	
	
	// ******************************************* adding external stubs ****************************
	
	
	
	//******************* printing ************************************
	
	cout<<"*************  writing files  *************"<<endl;
	
	
	ofstream out1("network.dat");
	for (int u=0; u<E_global.size(); u++) {

		set<int>::iterator itb=E_global[u].begin();
	
		while (itb!=E_global[u].end())
			out1<<u+1<<"\t"<<*(itb++)+1<<endl;
		
		

	}
		

	ofstream out2("community_first_level.dat");

	for (int i=0; i<member_list_global.size(); i++) {
		
		out2<<i+1<<"\t";
		for (int j=0; j<member_list_global[i].size(); j++)
			out2<<member_list_global[i][j]+1<<" ";
		out2<<endl;
	
	}

	
	ofstream out3("community_second_level.dat");

	for (int i=0; i<member_list_global_second_level.size(); i++) {
		
		out3<<i+1<<"\t"<<member_list_global_second_level[i][0]<<endl;
			
	}


	//******************* printing ************************************
	
	
	
	
	return 0;
	
}


