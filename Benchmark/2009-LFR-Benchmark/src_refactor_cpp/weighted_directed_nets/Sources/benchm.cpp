

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


#include "dir_benchm.cpp"


int print_network(deque<set<int> > & Ein, deque<set<int> > & Eout, const deque<deque<int> > & member_list, const deque<deque<int> > & member_matrix, 
	deque<int> & num_seq, deque<map <int, double > > & neigh_weigh_in, deque<map <int, double > > & neigh_weigh_out, double beta, double mu, double mu0) {

	
	int edges=0;

		
	int num_nodes=member_list.size();
	
	deque<double> double_mixing_in;
	for (int i=0; i<Ein.size(); i++) if(Ein[i].size()!=0) {
		
		double one_minus_mu = double(internal_kin(Ein, member_list, i))/Ein[i].size();
		
		double_mixing_in.push_back(fabs(1.- one_minus_mu));
		edges+=Ein[i].size();
		
	}
	
	deque<double> double_mixing_out;
	for (int i=0; i<Eout.size(); i++) if(Eout[i].size()!=0) {
		
		double one_minus_mu = double(internal_kin(Eout, member_list, i))/Eout[i].size();		
	
		double_mixing_out.push_back(fabs(1.- one_minus_mu));
		
		
	}

	
	
	//cout<<"\n----------------------------------------------------------"<<endl;
	//cout<<endl;
	
	
	double density=0; 
	double sparsity=0;
	
	for (int i=0; i<member_matrix.size(); i++) {

		double media_int=0;
		double media_est=0;
		
		for (int j=0; j<member_matrix[i].size(); j++) {
			
			
			double kinj = double(internal_kin_only_one(Ein[member_matrix[i][j]], member_matrix[i]));
			media_int+= kinj;
			media_est+=Ein[member_matrix[i][j]].size() - double(internal_kin_only_one(Ein[member_matrix[i][j]], member_matrix[i]));
					
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
	for (int u=0; u<Eout.size(); u++) {

		for(set<int>::iterator itb=Eout[u].begin(); itb!=Eout[u].end(); itb++)
			out1<<u+1<<"\t"<<*(itb)+1<<"\t"<<neigh_weigh_out[u][*(itb)]<<endl;
		

	}
		

	
	ofstream out2("community.dat");
	for (int i=0; i<member_list.size(); i++) {
		
		out2<<i+1<<"\t";
		for (int j=0; j<member_list[i].size(); j++)
			out2<<member_list[i][j]+1<<" ";
		out2<<endl;
	
	}

	cout<<"\n\n---------------------------------------------------------------------------"<<endl;
	
	
	cout<<"network of "<<num_nodes<<" vertices and "<<edges<<" edges"<<";\t average degree = "<<double(edges)/num_nodes<<endl;
	cout<<"\naverage mixing parameter (in-links): "<<average_func(double_mixing_in)<<" +/- "<<sqrt(variance_func(double_mixing_in))<<endl;
	cout<<"average mixing parameter (out-links): "<<average_func(double_mixing_out)<<" +/- "<<sqrt(variance_func(double_mixing_out))<<endl;
	cout<<"p_in: "<<density<<"\tp_out: "<<sparsity<<endl;

	
	
	ofstream statout("statistics.dat");
	
	deque<int> degree_seq_in;
	for (int i=0; i<Ein.size(); i++)
		degree_seq_in.push_back(Ein[i].size());
	
	deque<int> degree_seq_out;
	for (int i=0; i<Eout.size(); i++)
	degree_seq_out.push_back(Eout[i].size());

	statout<<"in-degree distribution (probability density function of the degree in logarithmic bins) "<<endl;
	log_histogram(degree_seq_in, statout, 10);
	statout<<"\nin-degree distribution (degree-occurrences) "<<endl;
	int_histogram(degree_seq_in, statout);
	statout<<endl<<"--------------------------------------"<<endl;
	
	
	statout<<"out-degree distribution (probability density function of the degree in logarithmic bins) "<<endl;
	log_histogram(degree_seq_out, statout, 10);
	statout<<"\nout-degree distribution (degree-occurrences) "<<endl;
	int_histogram(degree_seq_out, statout);
	statout<<endl<<"--------------------------------------"<<endl;
		
	statout<<"community distribution (size-occurrences)"<<endl;
	int_histogram(num_seq, statout);
	statout<<endl<<"--------------------------------------"<<endl;

	statout<<"mixing parameter (in-links)"<<endl;
	not_norm_histogram(double_mixing_in, statout, 20, 0, 0);
	statout<<endl<<"--------------------------------------"<<endl;
	
	statout<<"mixing parameter (out-links)"<<endl;
	not_norm_histogram(double_mixing_out, statout, 20, 0, 0);
	statout<<endl<<"--------------------------------------"<<endl;
	
	deque<double> inwij;
	deque<double> outwij;
	//deque<double> inkij;
	//deque<double> outkij;
	
	double csi=(1. - mu) / (1. - mu0);
	double csi2=mu /mu0;
	
	
	double tstrength=0;
	deque<double> one_minus_mu2;
	
	for(int i=0; i<neigh_weigh_in.size(); i++) {
		
		
		double internal_strength_i=0;
		double strength_i=0;
		
		for(map<int, double>::iterator itm = neigh_weigh_in[i].begin(); itm!=neigh_weigh_in[i].end(); itm++) {
			
			
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
		
		
		
		
		for(map<int, double>::iterator itm = neigh_weigh_out[i].begin(); itm!=neigh_weigh_out[i].end(); itm++) {
			
			
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
		
		one_minus_mu2.push_back(1 -internal_strength_i/strength_i);
		
	}
	
	
	//cout<<"average strength "<<tstrength / E.size()<<"\taverage internal strenght: "<<average_internal_strenght<<endl;
	cout<<"\naverage mixing parameter (weights): "<<average_func(one_minus_mu2)<<" +/- "<<sqrt(variance_func(one_minus_mu2))<<endl;	
	statout<<"mixing parameter (weights)"<<endl;
	not_norm_histogram(one_minus_mu2, statout, 20, 0, 0);
	statout<<endl<<"--------------------------------------"<<endl;

		
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




int check_weights(deque<map <int, double > > & neigh_weigh_in, deque<map <int, double > > & neigh_weigh_out, const deque<deque<int> > & member_list, 
	deque<deque<double> > & wished, deque<deque<double> > & factual, const double tot_var, double *strs) {

	
	
	
	double d1t=0;
	double d2t=0;
	double d3t=0;
	
	double var_check=0;
	
	for (int k=0; k<neigh_weigh_in.size(); k++) {
	
		
		double in_s=0;
		double out_s=0;
		
		
		
			
		for(map<int, double>::iterator itm=neigh_weigh_in[k].begin(); itm!=neigh_weigh_in[k].end(); itm++) {
			
			if(itm->second<0)
				cherr(333);
			
			if(fabs(itm->second - neigh_weigh_out[itm->first][k]) > 1e-7)
				cherr(111);
			
			if (they_are_mate(k, itm->first, member_list))
				in_s+=itm->second;
			else
				out_s+=itm->second;
		
		}
		
		for(map<int, double>::iterator itm=neigh_weigh_out[k].begin(); itm!=neigh_weigh_out[k].end(); itm++) {
			
			if(itm->second<0)
				cherr(333);
			
			if(fabs(itm->second - neigh_weigh_in[itm->first][k]) > 1e-7)
				cherr(111);
			
			if (they_are_mate(k, itm->first, member_list))
				in_s+=itm->second;
			else
				out_s+=itm->second;
		
		}

		
		//cout<<"start check"<<endl;
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



int propagate_one(deque<map <int, double > > & neighbors_weights, const deque<int> & VE, const deque<deque<int> > & member_list, 
	deque<deque<double> > & wished, deque<deque<double> > & factual, int i, double & tot_var, double *strs, const deque<int> & internal_kin_top, deque<map <int, double > > & others) {		
	
	
	double change=factual[i][2]/VE[i];
	
	
	
	double oldpartvar=0;
	for(map<int, double>::iterator itm=neighbors_weights[i].begin(); itm!=neighbors_weights[i].end(); itm++) if(itm->second + change > 0)
		for (int bw=0; bw<3; bw++) 
			oldpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);


	for (int bw=0; bw<3; bw++)
		oldpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);

	
	double newpartvar=0;

	for(map<int, double>::iterator itm=neighbors_weights[i].begin(); itm!=neighbors_weights[i].end(); itm++) if(itm->second + change > 0) {
	
		
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
		others[itm->first][i]+=change;
		

	}
	


	for (int bw=0; bw<3; bw++)
		newpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);
	
	
	
	tot_var+= newpartvar - oldpartvar;
		

	
	
	return 0;
	
		

}


int propagate_two(deque<map <int, double > > & neighbors_weights, const deque<int> & VE, const deque<deque<int> > & member_list, 
	deque<deque<double> > & wished, deque<deque<double> > & factual, int i, double & tot_var, double *strs, const deque<int> & internal_kin_top, deque<map <int, double > > & others) {


	int internal_neigh=internal_kin_top[i];


	if(internal_neigh!=0) {		// in this case I rewire the difference strength

		
		
		double changenn=(factual[i][0] - wished[i][0]);
				
		
		double oldpartvar=0;
		for(map<int, double>::iterator itm=neighbors_weights[i].begin(); itm!=neighbors_weights[i].end(); itm++) {
				
			
			if(they_are_mate(i, itm->first, member_list)) {
				
				double change = changenn/internal_neigh;
				
				if(itm->second - change > 0)
					for (int bw=0; bw<3; bw++) 
						oldpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);				
			
			} 
			
			else {
				
				double change = changenn/(VE[i] - internal_neigh);

				
				if(itm->second + change > 0)
					for (int bw=0; bw<3; bw++) 
						oldpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);

			}
		
		}
		
	
		for (int bw=0; bw<3; bw++)
			oldpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);

		
		double newpartvar=0;

		for(map<int, double>::iterator itm=neighbors_weights[i].begin(); itm!=neighbors_weights[i].end(); itm++) {
		
			
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
					others[itm->first][i]-=change;


				
				}
					
			}
				
			else {
				
				
				double change = changenn/(VE[i] - internal_neigh);
				
				if(itm->second + change > 0) {
					
					factual[itm->first][1]+=change;
					factual[itm->first][2]-=change;
					
					factual[i][1]+=change;
					factual[i][2]-=change;
					
					for (int bw=0; bw<3; bw++)
						newpartvar+= (factual[itm->first][bw] - wished[itm->first][bw]) * (factual[itm->first][bw] - wished[itm->first][bw]);
					
					
					itm->second+= change;
					others[itm->first][i]+=change;

					
				}
					
			}
			
							
	
		}
		
	
	
		for (int bw=0; bw<3; bw++)
			newpartvar+= (factual[i][bw] - wished[i][bw]) * (factual[i][bw] - wished[i][bw]);
		
		
		tot_var+=newpartvar - oldpartvar;
			
		
				

	
	
	
	}

	
	return 0;

}


int propagate(deque<map <int, double > > & neigh_weigh_in, deque<map <int, double > > & neigh_weigh_out, const deque<int> & VE, const deque<deque<int> > & member_list, 
	deque<deque<double> > & wished, deque<deque<double> > & factual, int i, double & tot_var, double *strs, const deque<int> & internal_kin_top) {
	
	
	
	
		
	
	propagate_one(neigh_weigh_in, VE, member_list, wished, factual, i, tot_var, strs, internal_kin_top, neigh_weigh_out);
	propagate_one(neigh_weigh_out, VE, member_list, wished, factual, i, tot_var, strs, internal_kin_top, neigh_weigh_in);
	propagate_two(neigh_weigh_in, VE, member_list, wished, factual, i, tot_var, strs, internal_kin_top, neigh_weigh_out);
	propagate_two(neigh_weigh_out, VE, member_list, wished, factual, i, tot_var, strs, internal_kin_top, neigh_weigh_in);

	//check_weights(neigh_weigh_in, neigh_weigh_out, member_list, wished, factual, tot_var, strs);
	
	
	return 0;
	
}
	
	

int weights(deque<set<int> > & ein, deque<set<int> > & eout, const deque<deque<int> > & member_list, const double beta, 
	const double mu, deque<map <int, double > > & neigh_weigh_in, deque<map <int, double > > & neigh_weigh_out) {

	
	
	double tstrength=0;
	
	deque<int> VE;							//VE is the degree of the nodes (in + out)
	deque<int> internal_kin_top;			//this is the internal degree of the nodes (in + out)
	
	
	for(int i=0; i<ein.size(); i++) {
		internal_kin_top.push_back(internal_kin(ein, member_list, i) + internal_kin(eout, member_list, i));
		VE.push_back(ein[i].size()+eout[i].size());
		tstrength+=pow(VE[i], beta);
	}
	
	
	
	double strs[VE.size()]; // strength of the nodes
	// build a matrix like this: deque < map <int, double > > each row corresponds to link - weights 

	
	
	for(int i=0; i<VE.size(); i++) {
		
		map<int, double> new_map;
		
		neigh_weigh_in.push_back(new_map);
		neigh_weigh_out.push_back(new_map);
		
		for (set<int>::iterator its=ein[i].begin(); its!=ein[i].end(); its++)
			neigh_weigh_in[i].insert(make_pair(*its, 0.));
		
		for (set<int>::iterator its=eout[i].begin(); its!=eout[i].end(); its++)
			neigh_weigh_out[i].insert(make_pair(*its, 0.));
		
		strs[i]=pow(double(VE[i]), beta);
		//cout<<VE[i]<<" "<<strs[i]<<endl;

		
	}
	
	
	
	
	deque<double> s_in_out_id_row(3);
	s_in_out_id_row[0]=0;
	s_in_out_id_row[1]=0;
	s_in_out_id_row[2]=0;
	
	
	deque<deque<double> > wished;	// 3 numbers for each node: internal, idle and extra strength. the sum of the three is strs[i]. wished is the theoretical, factual the factual one.
	deque<deque<double> > factual;
	
	
	for (int i=0; i<VE.size(); i++) {
		
		wished.push_back(s_in_out_id_row);
		factual.push_back(s_in_out_id_row);
		
	}
	
	
	
	double tot_var=0;
	

	for (int i=0; i<VE.size(); i++) {
		
		wished[i][0]=(1. -mu)*strs[i];
		wished[i][1]=mu*strs[i];
		
		factual[i][2]=strs[i];
		
		tot_var+= wished[i][0] * wished[i][0] + wished[i][1] * wished[i][1] + strs[i] * strs[i];
	
	}
	
	

	
	
	double precision = 1e-9;
	double precision2 = 1e-2;
	double not_better_than = pow(tstrength, 2) * precision;
	//cout<<"tot_var "<<tot_var<<";\tnot better "<<not_better_than<<endl;
	
	
	
	int step=0;
	
	while (true) {
	
		
		

		time_t t0=time(NULL);
		
		double pre_var=tot_var;
		
		
		for (int i=0; i<VE.size(); i++)
			propagate(neigh_weigh_in, neigh_weigh_out, VE, member_list, wished, factual, i, tot_var, strs, internal_kin_top);
		
		//check_weights(neigh_weigh_in, neigh_weigh_out, member_list, wished, factual, tot_var, strs);
		
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
	
	
	//check_weights(neigh_weigh_in, neigh_weigh_out, member_list, wished, factual, tot_var, strs);

	

	
	

	
	
	return 0;

}


int benchmark(bool excess, bool defect, int num_nodes, double  average_k, int  max_degree, double  tau, double  tau2, 
	double  mixing_parameter, double  mixing_parameter2, double  beta, int  overlapping_nodes, int  overlap_membership, int  nmin, int  nmax, bool  fixed_range) {	

	
	
	
	

	
	
	// it finds the minimum degree -----------------------------------------------------------------------

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
	//----------------------------------------------------------------------------------------------------
	
	
	deque <int> degree_seq_in;		//  degree sequence of the nodes (in-links)
	deque <int> degree_seq_out;		//  degree sequence of the nodes (out-links)
	deque <double> cumulative;
	powerlaw(max_degree, min_degree, tau, cumulative);
	
	for (int i=0; i<num_nodes; i++) {
		
		int nn=lower_bound(cumulative.begin(), cumulative.end(), ran4())-cumulative.begin()+min_degree;
		degree_seq_in.push_back(nn);
	
	}
	
	
	
	sort(degree_seq_in.begin(), degree_seq_in.end());
		
	int inarcs=deque_int_sum(degree_seq_in);
	compute_internal_degree_per_node(inarcs, degree_seq_in.size(), degree_seq_out);
	
	
		
	
	deque<deque<int> >  member_matrix;
	deque<int> num_seq;
	deque<int> internal_degree_seq_in;
	deque<int> internal_degree_seq_out;
	
	// ********************************			internal_degree and membership			***************************************************

	
	

	if(internal_degree_and_membership(mixing_parameter, overlapping_nodes, overlap_membership, num_nodes, member_matrix, excess, defect, degree_seq_in, degree_seq_out, num_seq, internal_degree_seq_in, internal_degree_seq_out, fixed_range, nmin, nmax, tau2)==-1)
		return -1;
	
	
	
	deque<set<int> > Ein;				// Ein is the adjacency matrix written in form of list of edges (in-links)
	deque<set<int> > Eout;				// Eout is the adjacency matrix written in form of list of edges (out-links)
	deque<deque<int> > member_list;		// row i cointains the memberships of node i
	deque<deque<int> > link_list_in;	// row i cointains degree of the node i respect to member_list[i][j]; there is one more number that is the external degree (in-links)
	deque<deque<int> > link_list_out;	// row i cointains degree of the node i respect to member_list[i][j]; there is one more number that is the external degree (out-links)

	
	
	cout<<"building communities... "<<endl;
	if(build_subgraphs(Ein, Eout, member_matrix, member_list, link_list_in, link_list_out, internal_degree_seq_in, degree_seq_in, internal_degree_seq_out, degree_seq_out, excess, defect)==-1)
		return -1;	
	

	
	cout<<"connecting communities... "<<endl;
	connect_all_the_parts(Ein, Eout, member_list, link_list_in, link_list_out);
	


	if(erase_links(Ein, Eout, member_list, excess, defect, mixing_parameter)==-1)
		return -1;
	
	
	
	//--------------------------------------------------------------------------------------
	
	
	deque<map <int, double > > neigh_weigh_in;
	deque<map <int, double > > neigh_weigh_out;

	cout<<"inserting weights..."<<endl;
	weights(Ein, Eout, member_list, beta, mixing_parameter2, neigh_weigh_in, neigh_weigh_out);
	
	
	

	cout<<"recording network..."<<endl;	
	print_network(Ein, Eout, member_list, member_matrix, num_seq, neigh_weigh_in, neigh_weigh_out, beta, mixing_parameter2, mixing_parameter);

	
	
	
		
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

	
	benchmark(p.excess, p.defect, p.num_nodes, p.average_k, p.max_degree, p.tau, p.tau2, p.mixing_parameter,  p.mixing_parameter2,  p.beta, p.overlapping_nodes, p.overlap_membership, p.nmin, p.nmax, p.fixed_range);	
		
	return 0;
	
}


