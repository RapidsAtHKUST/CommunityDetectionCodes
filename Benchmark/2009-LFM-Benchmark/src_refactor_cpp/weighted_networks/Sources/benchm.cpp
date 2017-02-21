



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
 *	Modified on 28/05/09                                                         *
 *	Collaborators: Santo Fortunato												 *
 *  Location: ISI foundation, Turin, Italy                                       *
 *	Project: Benchmarking community detection programs                           *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */


#include "binary_benchm.cpp"


int print_network(deque<set<int> > & E, const deque<deque<int> > & member_list, const deque<deque<int> > & member_matrix, 
	deque<int> & num_seq, deque<map <int, double > > & neigh_weigh, double beta, double mu, double mu0) {

	
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
	
	
	


	ofstream out1("network.dat");
	for (int u=0; u<E.size(); u++) {

		set<int>::iterator itb=E[u].begin();
	
		while (itb!=E[u].end())
			out1<<u+1<<"\t"<<*(itb++)+1<<"\t"<<neigh_weigh[u][*(itb)]<<endl;
		
		

	}
		

	
	ofstream out2("community.dat");

	for (int i=0; i<member_list.size(); i++) {
		
		out2<<i+1<<"\t";
		for (int j=0; j<member_list[i].size(); j++)
			out2<<member_list[i][j]+1<<" ";
		out2<<endl;
	
	}

	cout<<"\n\n---------------------------------------------------------------------------"<<endl;
	
	
	cout<<"network of "<<num_nodes<<" vertices and "<<edges/2<<" edges"<<";\t average degree = "<<double(edges)/num_nodes<<endl;
	cout<<"\naverage mixing parameter (topology): "<< average_func(double_mixing)<<" +/- "<<sqrt(variance_func(double_mixing))<<endl;
	cout<<"p_in: "<<density<<"\tp_out: "<<sparsity<<endl;

	
	
	ofstream statout("statistics.dat");
	
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

	statout<<"mixing parameter (topology)"<<endl;
	not_norm_histogram(double_mixing, statout, 20, 0, 0);
	statout<<endl<<"--------------------------------------"<<endl;
	
	
	//*
	
	deque<double> inwij;
	deque<double> outwij;
	//deque<double> inkij;
	//deque<double> outkij;
	
	double csi=(1. - mu) / (1. - mu0);
	double csi2=mu /mu0;
	
	
	double tstrength=0;
	deque<double> one_minus_mu2;
	
	for(int i=0; i<neigh_weigh.size(); i++) {
		
		
		double internal_strength_i=0;
		double strength_i=0;
		
		for(map<int, double>::iterator itm = neigh_weigh[i].begin(); itm!=neigh_weigh[i].end(); itm++) {
			
			
			if(they_are_mate(i, itm->first, member_list)) {
				
				inwij.push_back(itm->second);
				//inkij.push_back(csi * pow(E[i].size(), beta-1));
				internal_strength_i+=itm->second;

				
			}
			else {
				
				outwij.push_back(itm->second);
				//outkij.push_back(csi2 * pow(E[i].size(), beta-1));
			
			
			}
			
			tstrength+=itm->second;
			strength_i+=itm->second;
		
		
		}
		
		one_minus_mu2.push_back(1 - internal_strength_i/strength_i);
		
	}
	
	
	//cout<<"average strength "<<tstrength / E.size()<<"\taverage internal strenght: "<<average_internal_strenght<<endl;
	cout<<"\naverage mixing parameter (weights): "<<average_func(one_minus_mu2)<<" +/- "<<sqrt(variance_func(one_minus_mu2))<<endl;	
	statout<<"mixing parameter (weights)"<<endl;
	not_norm_histogram(one_minus_mu2, statout, 20, 0, 0);
	statout<<endl<<"--------------------------------------"<<endl;

	
	//cout<<" expected internal "<<tstrength * (1 - mu) / E.size()<<endl;
	//cout<<"internal links: "<<inwij.size()<<" external: "<<outwij.size()<<endl;
	
	/*
	ofstream hout1("inwij.dat");
	not_norm_histogram(inwij, hout1, 20, 0, 0);
	ofstream hout2("outwij.dat");
	not_norm_histogram(outwij, hout2, 20, 0, 0);
	ofstream hout3("corrin.dat");
	not_norm_histogram_correlated(inkij, inwij, hout3, 20, 0, 0);
	ofstream hout4("corrout.dat");
	not_norm_histogram_correlated(outkij, outwij, hout4, 20, 0, 0);
	
	//*/
	
	//*/
	
	cout<<"average weight of an internal link "<<average_func(inwij)<<" +/- "<<sqrt(variance_func(inwij))<<endl;
	cout<<"average weight of an external link "<<average_func(outwij)<<" +/- "<<sqrt(variance_func(outwij))<<endl;


	//cout<<"average weight of an internal link expected "<<tstrength / edges * (1. - mu) / (1. - mu0)<<endl;
	//cout<<"average weight of an external link expected "<<tstrength / edges * (mu) / (mu0)<<endl;
	
	
	statout<<"internal weights (weight-occurrences)"<<endl;
	not_norm_histogram(inwij, statout, 20, 0, 0);
	statout<<endl<<"--------------------------------------"<<endl;
	
	
	statout<<"external weights (weight-occurrences)"<<endl;
	not_norm_histogram(outwij, statout, 20, 0, 0);

	


	cout<<endl<<endl;

	return 0;

}



//*
int check_weights(deque<map <int, double > > & neigh_weigh, const deque<deque<int> > & member_list, 
	deque<deque<double> > & wished, deque<deque<double> > & factual, const double tot_var, double *strs) {

	
	
	
	double d1t=0;
	double d2t=0;
	double d3t=0;
	
	double var_check=0;
	
	for (int k=0; k<member_list.size(); k++) {
	
		
		double in_s=0;
		double out_s=0;
		
		
		
			
		for(map<int, double>::iterator itm=neigh_weigh[k].begin(); itm!=neigh_weigh[k].end(); itm++) {
			
			if(itm->second<0)
				cherr(333);
			
			if(fabs(itm->second - neigh_weigh[itm->first][k]) > 1e-7)
				cherr(111);
			
			if (they_are_mate(k, itm->first, member_list))
				in_s+=itm->second;
			else
				out_s+=itm->second;
		
		}
		
		
					
		if (fabs(in_s - factual[k][0]) > 1e-7)
			cherr(in_s - factual[k][0]);
		//cout<<"ok1"<<endl;
		
		if (fabs(out_s - factual[k][1]) > 1e-7)
			cherr(out_s - factual[k][1]);		
		//cout<<"ok2"<<endl;


		if (fabs(in_s + out_s + factual[k][2] - strs[k]) > 1e-7)
			cherr(in_s + out_s + factual[k][2] - strs[k]);
		//cout<<"ok3"<<endl;
		
		double d1=(in_s - wished[k][0]);
		double d2=(out_s - wished[k][1]);
		double d3=(strs[k] - in_s - out_s);
		var_check+= d1*d1 + d2*d2 + d3*d3;
		
		d1t+=d1*d1;
		d2t+=d2*d2;
		d3t+=d3*d3;
		
		
	
	}
	
		
	cout<<"tot_var "<<tot_var<<"\td1t "<<d1t<<"\td2t "<<d2t<<"\td3t "<<d3t<<endl;
	if (fabs(var_check - tot_var) > 1e-5)
		cerr<<"found this difference in check "<<fabs(var_check - tot_var)<<endl;
	else
		cout<<"ok: check passed"<<endl;

	// ----------------------------------------------------------------------- CHECK -----------------------------------------------
	// ----------------------------------------------------------------------- CHECK -----------------------------------------------
	
	


	return 0;

}


//*/



int propagate(deque<map <int, double > > & neigh_weigh, const deque<deque<int> > & member_list, 
	deque<deque<double> > & wished, deque<deque<double> > & factual, int i, double & tot_var, double *strs, const deque<int> & internal_kin_top) {
	
	
	
	
		
	
	
	{		// in this case I rewire the idle strength

		
		double change=factual[i][2]/neigh_weigh[i].size();
		
		
		
		
		
		double oldpartvar=0;
		for(map<int, double>::iterator itm=neigh_weigh[i].begin(); itm!=neigh_weigh[i].end(); itm++) if(itm->second + change > 0)
			for (int bw=0; bw<3; bw++) 
				oldpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);
	
	
		for (int bw=0; bw<3; bw++)
			oldpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);

		
		double newpartvar=0;

		for(map<int, double>::iterator itm=neigh_weigh[i].begin(); itm!=neigh_weigh[i].end(); itm++) if(itm->second + change > 0) {
		
			
			if (they_are_mate(i, itm->first, member_list)) {
					
				factual[itm->first][0]+=change;
				factual[itm->first][2]-=change;
					
				factual[i][0]+=change;
				factual[i][2]-=change;
					
			}
				
			else {
					
				factual[itm->first][1]+=change;
				factual[itm->first][2]-=change;
					
				factual[i][1]+=change;
				factual[i][2]-=change;

					
			}
			
			for (int bw=0; bw<3; bw++)
				newpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);
			
			
			
			itm->second+= change;
			neigh_weigh[itm->first][i]+=change;
			
	
		}
		
	
	
		for (int bw=0; bw<3; bw++)
			newpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);
		
		
		
		tot_var+= newpartvar - oldpartvar;
			
	
		
		
			
	
	}
	
	
	int internal_neigh=internal_kin_top[i];


	if(internal_neigh!=0) {		// in this case I rewire the difference strength

		
		
		double changenn=(factual[i][0] - wished[i][0]);
				
		
		double oldpartvar=0;
		for(map<int, double>::iterator itm=neigh_weigh[i].begin(); itm!=neigh_weigh[i].end(); itm++) {
				
			
			if(they_are_mate(i, itm->first, member_list)) {
				
				double change = changenn/internal_neigh;
				
				if(itm->second - change > 0)
					for (int bw=0; bw<3; bw++) 
						oldpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);				
			
			} 
			
			else {
				
				double change = changenn/(neigh_weigh[i].size() - internal_neigh);

				
				if(itm->second + change > 0)
					for (int bw=0; bw<3; bw++) 
						oldpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);

			}
		
		}
		
	
		for (int bw=0; bw<3; bw++)
			oldpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);

		
		double newpartvar=0;

		for(map<int, double>::iterator itm=neigh_weigh[i].begin(); itm!=neigh_weigh[i].end(); itm++) {
		
			
			if (they_are_mate(i, itm->first, member_list)) {
				
				double change = changenn/internal_neigh;

				
				if(itm->second - change > 0) {
					
					factual[itm->first][0]-=change;
					factual[itm->first][2]+=change;
					
					factual[i][0]-=change;
					factual[i][2]+=change;
					
					for (int bw=0; bw<3; bw++)
						newpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);
					
					
					itm->second-= change;
					neigh_weigh[itm->first][i]-=change;


				
				}
					
			}
				
			else {
				
				
				double change = changenn/(neigh_weigh[i].size() - internal_neigh);
				
				if(itm->second + change > 0) {
					
					factual[itm->first][1]+=change;
					factual[itm->first][2]-=change;
					
					factual[i][1]+=change;
					factual[i][2]-=change;
					
					for (int bw=0; bw<3; bw++)
						newpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);
					
					
					itm->second+= change;
					neigh_weigh[itm->first][i]+=change;


					
				}
					
			}
			
							
	
		}
		
	
	
		for (int bw=0; bw<3; bw++)
			newpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);
		
		
		tot_var+=newpartvar - oldpartvar;
			
		
				

	
	
	
	}
	
	
	//check_weights(neigh_weigh, member_list, wished, factual, tot_var, strs);
	
	
	return 0;
	
}
	
	

int weights(deque<set<int> > & en, const deque<deque<int> > & member_list, const double beta, const double mu, deque<map <int, double > > & neigh_weigh) {

	
	
	double tstrength=0;
	
	
	for(int i=0; i<en.size(); i++)
		tstrength+=pow(en[i].size(), beta);
		
	
	
	double strs[en.size()]; // strength of the nodes
	// build a matrix like this: deque < map <int, double > > each row corresponds to link - weights 

	
	
	for(int i=0; i<en.size(); i++) {
		
		map<int, double> new_map;
		neigh_weigh.push_back(new_map);
		
		for (set<int>::iterator its=en[i].begin(); its!=en[i].end(); its++)
			neigh_weigh[i].insert(make_pair(*its, 0.));
		
		strs[i]=pow(double(en[i].size()), beta);
		
	}
	
	
	
	
	deque<double> s_in_out_id_row(3);
	s_in_out_id_row[0]=0;
	s_in_out_id_row[1]=0;
	s_in_out_id_row[2]=0;
	
	
	deque<deque<double> > wished;	// 3 numbers for each node: internal, idle and extra strength. the sum of the three is strs[i]. wished is the theoretical, factual the factual one.
	deque<deque<double> > factual;
	
	
	
	for (int i=0; i<en.size(); i++) {
		
		wished.push_back(s_in_out_id_row);
		factual.push_back(s_in_out_id_row);
		
	}
	
	
	
	double tot_var=0;
	

	for (int i=0; i<en.size(); i++) {
		
		wished[i][0]=(1. -mu)*strs[i];
		wished[i][1]=mu*strs[i];
		
		factual[i][2]=strs[i];
		
		tot_var+= wished[i][0] * wished[i][0] + wished[i][1] * wished[i][1] + strs[i] * strs[i];
	
	}
	
	
	deque<int> internal_kin_top;
	for(int i=0; i<en.size(); i++)
		internal_kin_top.push_back(internal_kin(en, member_list, i));

	
	
	double precision = 1e-9;
	double precision2 = 1e-2;
	double not_better_than = pow(tstrength, 2) * precision;
	//cout<<"tot_var "<<tot_var<<";\tnot better "<<not_better_than<<endl;
	
	
	
	int step=0;
	
	while (true) {
	
		
		

		time_t t0=time(NULL);
		
		double pre_var=tot_var;
		
		
		for (int i=0; i<en.size(); i++) 
			propagate(neigh_weigh, member_list, wished, factual, i, tot_var, strs, internal_kin_top);
		
		
		//check_weights(neigh_weigh, member_list, wished, factual, tot_var, strs);
		
		double relative_improvement=double(pre_var - tot_var)/pre_var;		
		//cout<<"tot_var "<<tot_var<<"\trelative improvement: "<<relative_improvement<<endl;

		if (tot_var<not_better_than)
				break;
		
		if (relative_improvement < precision2)
			break;
		
		time_t t1= time(NULL);
		int deltat= t1 - t0;
		
		/*
		if(step%2==0 && deltat !=0)		
			cout<<"About "<<cast_int((log(not_better_than) -log(tot_var)) / log(1. - relative_improvement)) * deltat<<" secs..."<<endl;
		*/
		step++;
		
	}
	
	
	//check_weights(neigh_weigh, member_list, wished, factual, tot_var, strs);

	

	
	

	
	
	return 0;

}


int benchmark(bool excess, bool defect, int num_nodes, double  average_k, int  max_degree, double  tau, double  tau2, 
	double  mixing_parameter, double  mixing_parameter2, double  beta, int  overlapping_nodes, int  overlap_membership, int  nmin, int  nmax, bool  fixed_range, double ca) {	

	
	
	
	

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
	
	
	deque<deque<int> >  member_matrix;
	deque<int> num_seq;
	deque<int> internal_degree_seq;
	
	// ********************************			internal_degree and membership			***************************************************

	
	

	if(internal_degree_and_membership(mixing_parameter, overlapping_nodes, overlap_membership, num_nodes, member_matrix, excess, defect, degree_seq, num_seq, internal_degree_seq, fixed_range, nmin, nmax, tau2)==-1)
		return -1;
	
	
	
		
	
	
	
	deque<set<int> > E;					// E is the adjacency matrix written in form of list of edges
	deque<deque<int> > member_list;		// row i cointains the memberships of node i
	deque<deque<int> > link_list;		// row i cointains degree of the node i respect to member_list[i][j]; there is one more number that is the external degree

	
	
	cout<<"building communities... "<<endl;
	if(build_subgraphs(E, member_matrix, member_list, link_list, internal_degree_seq, degree_seq, excess, defect)==-1)
		return -1;	
	




	cout<<"connecting communities... "<<endl;
	connect_all_the_parts(E, member_list, link_list);
	



	if(erase_links(E, member_list, excess, defect, mixing_parameter)==-1)
		return -1;
	

	if(ca!=unlikely) {
		cout<<"trying to approach an average clustering coefficient ... "<<ca<<endl;
		cclu(E, member_list, member_matrix, ca);
	}

	
	deque<map <int, double > > neigh_weigh;

	cout<<"inserting weights..."<<endl;
	weights(E, member_list, beta, mixing_parameter2, neigh_weigh);
	
	
	

	cout<<"recording network..."<<endl;	
	print_network(E, member_list, member_matrix, num_seq, neigh_weigh, beta, mixing_parameter2, mixing_parameter);

	
	
	
		
	return 0;
	
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



int main(int argc, char * argv[]) {
	
		
	srand_file();
	Parameters p;
	if(set_parameters(argc, argv, p)==false) {
		
		if (argc>1)
			cerr<<"Please, look at ReadMe.txt..."<<endl;
		
		return -1;
	
	}
	
	
	erase_file_if_exists("network.dat");
	erase_file_if_exists("community.dat");
	erase_file_if_exists("statistics.dat");

	
	benchmark(p.excess, p.defect, p.num_nodes, p.average_k, p.max_degree, p.tau, p.tau2, p.mixing_parameter,  p.mixing_parameter2,  p.beta, p.overlapping_nodes, p.overlap_membership, p.nmin, p.nmax, p.fixed_range, p.clustering_coeff);	
		
	return 0;
	
}


