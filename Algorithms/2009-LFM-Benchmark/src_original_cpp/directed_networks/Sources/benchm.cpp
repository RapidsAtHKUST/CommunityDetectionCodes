



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



#include "./standard_include.cpp"
#define unlikely -214741

#include "set_parameters.cpp"








/*
void printmcs(deque<set<int> > & A, deque<set<int> > & B) {

	for(int i=0; i<A.size(); i++) {
		
		for(set<int>::iterator its=A[i].begin(); its!=A[i].end(); its++) 
			if(*its==i)
				cherr(2111);
			
		
	
	}
	
	
	for(int i=0; i<A.size(); i++) {
		
		for(set<int>::iterator its=A[i].begin(); its!=A[i].end(); its++) 
			if(B[*its].find(i)==B[*its].end())
				cherr(2111);
				
	}

	
	cout<<"printmcs passed"<<endl;


}

//*/


// it computes the sum of a deque<int>

int deque_int_sum(const deque<int> & a) {
	
	int s=0;
	for(int i=0; i<a.size(); i++)
		s+=a[i];

	return s;
}

// it computes the integral of a power law
double integral (double a, double b) {

	
	if (fabs(a+1.)>1e-10)
		return (1./(a+1.)*pow(b, a+1.));
	
	
	else
		return (log(b));

}


// it returns the average degree of a power law
double average_degree(const double &dmax, const double &dmin, const double &gamma) {

	return (1./(integral(gamma, dmax)-integral(gamma, dmin)))*(integral(gamma+1, dmax)-integral(gamma+1, dmin));

}


//bisection method to find the inferior limit, in order to have the expected average degree
double solve_dmin(const double& dmax, const double &dmed, const double &gamma) {
	
	double dmin_l=1;
	double dmin_r=dmax;
	double average_k1=average_degree(dmin_r, dmin_l, gamma);
	double average_k2=dmin_r;
	
	
	if ((average_k1-dmed>0) || (average_k2-dmed<0)) {
		
		cerr<<"\n***********************\nERROR: the average degree is out of range:";
		
		if (average_k1-dmed>0) {
			cerr<<"\nyou should increase the average degree (bigger than "<<average_k1<<")"<<endl; 
			cerr<<"(or decrease the maximum degree...)"<<endl;
		}
		
		if (average_k2-dmed<0) {
			cerr<<"\nyou should decrease the average degree (smaller than "<<average_k2<<")"<<endl; 
			cerr<<"(or increase the maximum degree...)"<<endl;
		}
		
		return -1;	
	}
	
		
	while (fabs(average_k1-dmed)>1e-7) {
		
		double temp=average_degree(dmax, ((dmin_r+dmin_l)/2.), gamma);
		if ((temp-dmed)*(average_k2-dmed)>0) {
			
			average_k2=temp;
			dmin_r=((dmin_r+dmin_l)/2.);
		
		}
		else {
			
			average_k1=temp;
			dmin_l=((dmin_r+dmin_l)/2.);
			
		
		}
			
		

	
	}
	
	return dmin_l;
}



// it computes the correct (i.e. discrete) average of a power law
double integer_average (int n, int min, double tau) {
	
	double a=0;

	for (double h=min; h<n+1; h++)
		a+= pow((1./h),tau);
	
	
	double pf=0;
	for(double i=min; i<n+1; i++)
		pf+=1/a*pow((1./(i)),tau)*i;
	
	return pf;

}



// this function changes the community sizes merging the smallest communities
int change_community_size(deque<int> &seq) {

	
			
	if (seq.size()<=2)
		return -1;
	
	int min1=0;
	int min2=0;
	
	for (int i=0; i<seq.size(); i++)		
		if (seq[i]<=seq[min1])
			min1=i;
	
	if (min1==0)
		min2=1;
	
	for (int i=0; i<seq.size(); i++)		
		if (seq[i]<=seq[min2] && seq[i]>seq[min1])
			min2=i;
	

	
	seq[min1]+=seq[min2];
	
	int c=seq[0];
	seq[0]=seq[min2];
	seq[min2]=c;
	seq.pop_front();
	
	
	return 0;
}









int build_bipartite_network(deque<deque<int> >  & member_matrix, const deque<int> & member_numbers, const deque<int> &num_seq) {
    
	
	
	// this function builds a bipartite network with num_seq and member_numbers which are the degree sequences. in member matrix links of the communities are stored
	// this means member_matrix has num_seq.size() rows and each row has num_seq[i] elements
	
	
	
	deque<set<int> > en_in;			// this is the Ein of the subgraph
	deque<set<int> > en_out;		// this is the Eout of the subgraph
	
	
	{
    set<int> first;
    for(int i=0; i<member_numbers.size(); i++) {
        en_in.push_back(first);
    }
	}
	
	{
    set<int> first;
    for(int i=0; i<num_seq.size(); i++) {
        en_out.push_back(first);
    }
	}
    
	
	
	multimap <int, int> degree_node_out;
	deque<pair<int, int> > degree_node_in;
	
	for(int i=0; i<num_seq.size(); i++)
		degree_node_out.insert(make_pair(num_seq[i], i));
	
	for(int i=0; i<member_numbers.size(); i++)
		degree_node_in.push_back(make_pair(member_numbers[i], i));
	
	
	sort(degree_node_in.begin(), degree_node_in.end());
	
	
	
	deque<pair<int, int> >::iterator itlast = degree_node_in.end();
	
	/*
     for (int i=0; i<degree_node_in.size(); i++)
     cout<<degree_node_in[i].first<<" "<<degree_node_in[i].second<<endl;
     */
	
	
	
	while (itlast != degree_node_in.begin()) {
		
		itlast--;
		
		
		multimap <int, int>::iterator itit= degree_node_out.end();
		deque <multimap<int, int>::iterator> erasenda;
		
		for (int i=0; i<itlast->first; i++) {
			
			if(itit!=degree_node_out.begin()) {
				
				itit--;
				
                
				en_in[itlast->second].insert(itit->second);
				en_out[itit->second].insert(itlast->second);
                
				erasenda.push_back(itit);
				
			}
			
			else
				return -1;
            
		}
		
		
		//cout<<"degree node out before"<<endl;
		//prints(degree_node_out);
		
		for (int i=0; i<erasenda.size(); i++) {
			
			
			if(erasenda[i]->first>1)
				degree_node_out.insert(make_pair(erasenda[i]->first - 1, erasenda[i]->second));
            
			
			degree_node_out.erase(erasenda[i]);
            
			
            
		}
		
		//cout<<"degree node out after"<<endl;
		//prints(degree_node_out);
		
	}
	
	
	// this is to randomize the subgraph -------------------------------------------------------------------
    
    
    deque<int> degree_list;
    for(int kk=0; kk<member_numbers.size(); kk++)
        for(int k2=0; k2<member_numbers[kk]; k2++)
            degree_list.push_back(kk); 
    
	
	for(int run=0; run<10; run++) for(int node_a=0; node_a<num_seq.size(); node_a++) for(int krm=0; krm<en_out[node_a].size(); krm++) {
        
        int random_mate=degree_list[irand(degree_list.size()-1)];
		
		if (en_out[node_a].find(random_mate)==en_out[node_a].end()) {
			
			deque <int> external_nodes;
			for (set<int>::iterator it_est=en_out[node_a].begin(); it_est!=en_out[node_a].end(); it_est++)
				external_nodes.push_back(*it_est);
            
            
			int	old_node=external_nodes[irand(external_nodes.size()-1)];
            
			
			deque <int> not_common;
			for (set<int>::iterator it_est=en_in[random_mate].begin(); it_est!=en_in[random_mate].end(); it_est++)
				if (en_in[old_node].find(*it_est)==en_in[old_node].end())
					not_common.push_back(*it_est);
            
			
			if (not_common.empty())
				break;
            
			int node_h=not_common[irand(not_common.size()-1)];
			
			
			en_out[node_a].insert(random_mate);
			en_out[node_a].erase(old_node);
			
			en_in[old_node].insert(node_h);
			en_in[old_node].erase(node_a);
			
			en_in[random_mate].insert(node_a);
			en_in[random_mate].erase(node_h);
			
			en_out[node_h].erase(random_mate);
			en_out[node_h].insert(old_node);
			
            
		}	
	}
    
	
	
	member_matrix.clear();
	deque <int> first;
	
	for (int i=0; i<en_out.size(); i++) { 
		
		member_matrix.push_back(first);
		for (set<int>::iterator its=en_out[i].begin(); its!=en_out[i].end(); its++)
			member_matrix[i].push_back(*its);
        
	}
	
	
	return 0;
    
    
}



int internal_degree_and_membership (double mixing_parameter, int overlapping_nodes, int max_mem_num, int num_nodes, deque<deque<int> >  & member_matrix, 
bool excess, bool defect,  deque<int> & degree_seq_in, deque<int> & degree_seq_out, deque<int> &num_seq, deque<int> &internal_degree_seq_in, deque<int> &internal_degree_seq_out, bool fixed_range, int nmin, int nmax, double tau2) {
	
	
	
	
	if(num_nodes< overlapping_nodes) {
		
		cerr<<"\n***********************\nERROR: there are more overlapping nodes than nodes in the whole network! Please, decrease the former ones or increase the latter ones"<<endl;
		return -1;
	}
	
	
	// 
	member_matrix.clear();
	internal_degree_seq_in.clear();
	
	deque<double> cumulative;
	
	// it assigns the internal degree to each node -------------------------------------------------------------------------
	int max_degree_actual=0;		// maximum internal degree

	for (int i=0; i<degree_seq_in.size(); i++) {
		
		double interno=(1-mixing_parameter)*degree_seq_in[i];
		int int_interno=int(interno);
		
		
		if (ran4()<(interno-int_interno))
			int_interno++;
		
		if (excess) {
			
			while (   (  double(int_interno)/degree_seq_in[i] < (1-mixing_parameter) )  &&   (int_interno<degree_seq_in[i])   )
				int_interno++;
				
		
		}
		
		
		if (defect) {
			
			while (   (  double(int_interno)/degree_seq_in[i] > (1-mixing_parameter) )  &&   (int_interno>0)   )
				int_interno--;
				
		
		}

		
		
		
		internal_degree_seq_in.push_back(int_interno);
		
		
		if (int_interno>max_degree_actual)
			max_degree_actual=int_interno;
		
			
	}
	
	for (int i=0; i<degree_seq_out.size(); i++) {
		
		double interno=(1-mixing_parameter)*degree_seq_out[i];
		int int_interno=int(interno);
		
		
		if (ran4()<(interno-int_interno))
			int_interno++;
		
		if (excess) {
			
			while (   (  double(int_interno)/degree_seq_out[i] < (1-mixing_parameter) )  &&   (int_interno<degree_seq_out[i])   )
				int_interno++;
				
		
		}
		
		
		if (defect) {
			
			while (   (  double(int_interno)/degree_seq_out[i] > (1-mixing_parameter) )  &&   (int_interno>0)   )
				int_interno--;
				
		
		}

		
		internal_degree_seq_out.push_back(int_interno);
			
	}
	
	// it assigns the community size sequence -----------------------------------------------------------------------------
	
	powerlaw(nmax, nmin, tau2, cumulative);
	
	
	if (num_seq.empty()) {
		
		int _num_=0;
		if (!fixed_range && (max_degree_actual+1)>nmin) {
		
			_num_=max_degree_actual+1;			// this helps the assignment of the memberships (it assures that at least one module is big enough to host each node)
			num_seq.push_back(max_degree_actual+1);
		
		}
		
		
		while (true) {
			
			
			int nn=lower_bound(cumulative.begin(), cumulative.end(), ran4())-cumulative.begin()+nmin;
			
			if (nn+_num_<=num_nodes + overlapping_nodes * (max_mem_num-1) ) {
				
				num_seq.push_back(nn);				
				_num_+=nn;
			
			}
			else
				break;
			
			
		}
		
		num_seq[min_element(num_seq.begin(), num_seq.end()) - num_seq.begin()]+=num_nodes + overlapping_nodes * (max_mem_num-1) - _num_;
		
	}
	
	
	//cout<<"num_seq"<<endl;
	//prints(num_seq);
	
	int ncom=num_seq.size();
	
	//cout<<"\n----------------------------------------------------------"<<endl;


	deque<int> member_numbers;
	for(int i=0; i<overlapping_nodes; i++)
		member_numbers.push_back(max_mem_num);
	for(int i=overlapping_nodes; i<degree_seq_in.size(); i++)
		member_numbers.push_back(1);
	
	//prints(member_numbers);
	//prints(num_seq);
	
	if(build_bipartite_network(member_matrix, member_numbers, num_seq)==-1) {
		
		cerr<<"it seems that the overlapping nodes need more communities that those I provided. Please increase the number of communities or decrease the number of overlapping nodes"<<endl;
		return -1;			
	
	}

	
	
	//printm(member_matrix);
	
	//cout<<"degree_seq_in"<<endl;
	//prints(degree_seq_in);
	
	//cout<<"internal_degree_seq_in"<<endl;
	//prints(internal_degree_seq_in);

	deque<int> available;
	for (int i=0; i<num_nodes; i++)
		available.push_back(0);
	
	for (int i=0; i<member_matrix.size(); i++) {
		for (int j=0; j<member_matrix[i].size(); j++)
			available[member_matrix[i][j]]+=member_matrix[i].size()-1;
	}
	
	//cout<<"available"<<endl;
	//prints(available);
	
	
	deque<int> available_nodes;
	for (int i=0; i<num_nodes; i++)
		available_nodes.push_back(i);
	
	
	deque<int> map_nodes;				// in the position i there is the new name of the node i
	for (int i=0; i<num_nodes; i++)
		map_nodes.push_back(0);

	
	for (int i=degree_seq_in.size()-1; i>=0; i--) {
		
		int & degree_here=internal_degree_seq_in[i];
		int try_this = irand(available_nodes.size()-1);
		
		int kr=0;
		while (internal_degree_seq_in[i] > available[available_nodes[try_this]]) {
		
			kr++;
			try_this = irand(available_nodes.size()-1);
			if(kr==3*num_nodes) {
			
				if(change_community_size(num_seq)==-1) {
					
					cerr<<"\n***********************\nERROR: this program needs more than one community to work fine"<<endl;
					return -1;
				
				}
				
				cout<<"it took too long to decide the memberships; I will try to change the community sizes"<<endl;

				cout<<"new community sizes"<<endl;
				for (int i=0; i<num_seq.size(); i++)
					cout<<num_seq[i]<<" ";
				cout<<endl<<endl;
				
				return (internal_degree_and_membership(mixing_parameter, overlapping_nodes, max_mem_num, num_nodes, member_matrix, excess, defect, degree_seq_in, degree_seq_out, num_seq, internal_degree_seq_in, internal_degree_seq_out, fixed_range, nmin, nmax, tau2));
			
			
			}
			
			
		}
		
		
		
		map_nodes[available_nodes[try_this]]=i;
		
		available_nodes[try_this]=available_nodes[available_nodes.size()-1];
		available_nodes.pop_back();
		
	
	
	}
	
	
	for (int i=0; i<member_matrix.size(); i++) {
		for (int j=0; j<member_matrix[i].size(); j++)
			member_matrix[i][j]=map_nodes[member_matrix[i][j]];	
	}
	
	
	
	for (int i=0; i<member_matrix.size(); i++)
		sort(member_matrix[i].begin(), member_matrix[i].end());

		
	return 0;

}



int compute_internal_degree_per_node(int d, int m, deque<int> & a) {
	
	
	// d is the internal degree
	// m is the number of memebership 
	
	a.clear();
	int d_i= d/m;
	for (int i=0; i<m; i++)
		a.push_back(d_i);
		
	for(int i=0; i<d%m; i++)
		a[i]++;
	
	
	

	return 0;

}



/*
int check_link_list(const deque<deque<int> > & link_list_in, const deque<int> & degree_seq_in) {

	
	for (int i=0; i<link_list_in.size(); i++) {
	
		int s=0;
		for (int j=0; j<link_list_in[i].size(); j++)
			s+=link_list_in[i][j];
		
		if(s!=degree_seq_in[i]) {
			
			int ok;
			cerr<<"wrong link list"<<endl;
			cin>>ok;
		
		}
		
		
	
	
	}




}

*/


int build_subgraph(deque<set<int> > & Ein, deque<set<int> > & Eout, const deque<int> & nodes, const deque<int> & d_in, const deque<int> & d_out) {
	
	
	/*
	cout<<"nodes"<<endl;
	prints(nodes);
	
	cout<<"degrees"<<endl;
	prints(d_in);
	
	cout<<"d_out"<<endl;
	prints(d_out);
	//*/
	
	if(d_in.size()<3) {
		
		cerr<<"it seems that some communities should have only 2 nodes! This does not make much sense (in my opinion) Please change some parameters!"<<endl;
		return -1;
	
	}
	
	// this function is to build a network with the labels stored in nodes and the degree seq in degrees (correspondence is based on the vectorial index)
	// the only complication is that you don't want the nodes to have neighbors they already have
	
	
	
	// labels will be placed in the end
	deque<set<int> > en_in;			// this is the Ein of the subgraph
	deque<set<int> > en_out;		// this is the Eout of the subgraph
	
	
	{
		set<int> first;
		for(int i=0; i<nodes.size(); i++) {
			en_in.push_back(first);
			en_out.push_back(first);
		}
	}
	
	
	
	multimap <int, int> degree_node_out;
	deque<pair<int, int> > degree_node_in;
	
	for(int i=0; i<d_out.size(); i++)
		degree_node_out.insert(make_pair(d_out[i], i));
		
		
	deque<int> fakes;
	for(int i=0; i<d_in.size(); i++)
		fakes.push_back(i);
	
	shuffle_s(fakes);
	

	deque<int> antifakes(fakes.size());
	for(int i=0; i<d_in.size(); i++)
		antifakes[fakes[i]]=i;
	
	
	
	for(int i=0; i<d_in.size(); i++)
		degree_node_in.push_back(make_pair(d_in[i], fakes[i]));
	

	sort(degree_node_in.begin(), degree_node_in.end());
	

	for(int i=0; i<d_in.size(); i++)
		degree_node_in[i].second=antifakes[degree_node_in[i].second];

	
		
	
	
	deque<pair<int, int> >::iterator itlast = degree_node_in.end();
	
	/*
	for (int i=0; i<degree_node_in.size(); i++)
		cout<<degree_node_in[i].first<<" "<<degree_node_in[i].second<<endl;
	//*/
	
	deque<int> self_loop;
	
	int inserted=0;
	
	while (itlast != degree_node_in.begin()) {
		
		itlast--;
		
		
		multimap <int, int>::iterator itit= degree_node_out.end();
		deque <multimap<int, int>::iterator> erasenda;
		
		for (int i=0; i<itlast->first; i++) {
			
			if(itit!=degree_node_out.begin()) {
				
				itit--;
				
				if (itit->second!=itlast->second) {
					
					en_in[itlast->second].insert(itit->second);
					en_out[itit->second].insert(itlast->second);
					inserted++;
					
				}
				else
					self_loop.push_back(itlast->second);
				

				erasenda.push_back(itit);
				
			}
			
			else
				break;
		
		}
		
		
		//cout<<"degree node out before"<<endl;
		//prints(degree_node_out);
		
		for (int i=0; i<erasenda.size(); i++) {
			
			
			if(erasenda[i]->first>1)
				degree_node_out.insert(make_pair(erasenda[i]->first - 1, erasenda[i]->second));
	
			
			degree_node_out.erase(erasenda[i]);

			
		
		}
		
		//cout<<"degree node out after"<<endl;
		//prints(degree_node_out);
		
	}
	
	//cout<<inserted<<"<------ inserted"<<endl; 
	
	//cout<<"left "<<degree_node_out.size()<<endl;
	//cout<<"self loops "<<self_loop.size()<<endl;
    
    deque<int> degree_list_in;
    for(int kk=0; kk<d_in.size(); kk++)
        for(int k2=0; k2<d_in[kk]; k2++)
            degree_list_in.push_back(kk); 

    
	int not_done=0;
	
	for(int i=0; i<self_loop.size(); i++) {
	
		int node=self_loop[i];

		int stopper=d_in.size()*d_in.size();
		int stop=0;
		
		//cout<<"node "<<nodes[node]<<endl;
		
		bool breaker=false;

		while (stop++ < stopper) {
			
			
			while(true) {
				
				
				int random_mate=degree_list_in[irand(degree_list_in.size()-1)];
				if(random_mate==node || en_in[node].find(random_mate)!=en_in[node].end())
					break;
				
				deque <int> not_common;
				for (set<int>::iterator it_est=en_out[random_mate].begin(); it_est!=en_out[random_mate].end(); it_est++)
					if (en_out[node].find(*it_est)==en_out[node].end())
						not_common.push_back(*it_est);
					
			
				if (not_common.empty())
					break;
				
				int random_neigh=not_common[irand(not_common.size()-1)];

				
				en_out[node].insert(random_neigh);
				en_in[node].insert(random_mate);
				
				
				en_in[random_neigh].insert(node);
				en_in[random_neigh].erase(random_mate);
				
				en_out[random_mate].insert(node);
				en_out[random_mate].erase(random_neigh);
				
				breaker=true;
				break;
			
			
			}
			
			if(breaker)
				break;
			
		
		}
		
		if(!breaker)
			not_done++;
		
		
	}
	
	//cout<<"not done "<<not_done<<endl;
	
	// this is to randomize the subgraph -------------------------------------------------------------------
	
	for(int run=0; run<10; run++) for(int node_a=0; node_a<d_in.size(); node_a++) for(int krm=0; krm<en_out[node_a].size(); krm++) {
				
				
		int random_mate=degree_list_in[irand(degree_list_in.size()-1)];
		while (random_mate==node_a)
			random_mate=degree_list_in[irand(degree_list_in.size()-1)];
        
		
		if (en_out[node_a].find(random_mate)==en_out[node_a].end()) {
			
			deque <int> external_nodes;
			for (set<int>::iterator it_est=en_out[node_a].begin(); it_est!=en_out[node_a].end(); it_est++)
				external_nodes.push_back(*it_est);
						
										
			int	old_node=external_nodes[irand(external_nodes.size()-1)];
					
			
			deque <int> not_common;
			for (set<int>::iterator it_est=en_in[random_mate].begin(); it_est!=en_in[random_mate].end(); it_est++)
				if ((old_node!=(*it_est)) && (en_in[old_node].find(*it_est)==en_in[old_node].end()))
					not_common.push_back(*it_est);
					
			
			if (not_common.empty())
				break;
				
			int node_h=not_common[irand(not_common.size()-1)];
			
			
			en_out[node_a].insert(random_mate);
			en_out[node_a].erase(old_node);
			
			en_in[old_node].insert(node_h);
			en_in[old_node].erase(node_a);
			
			en_in[random_mate].insert(node_a);
			en_in[random_mate].erase(node_h);
			
			en_out[node_h].erase(random_mate);
			en_out[node_h].insert(old_node);
			

		}
	
	
	}

	
	
	// now I try to insert the new links into the already done network. If some multiple links come out, I try to rewire them
	
	deque < pair<int, int> > multiple_edge;
	for (int i=0; i<en_in.size(); i++) {
		
		for(set<int>::iterator its=en_in[i].begin(); its!=en_in[i].end(); its++) {
		
			bool already = !(Ein[nodes[i]].insert(nodes[*its]).second) ;		// true is the insertion didn't take place
			if (already)
				multiple_edge.push_back(make_pair(nodes[i], nodes[*its]));			
			else
				Eout[nodes[*its]].insert(nodes[i]);
		
		}
	
	
	}
	
	
	//cout<<"multiple "<<multiple_edge.size()<<endl;
	
	for (int i=0; i<multiple_edge.size(); i++) {
		
		
		int &a = multiple_edge[i].first;
		int &b = multiple_edge[i].second;
		
	
		// now, I'll try to rewire this multiple link among the nodes stored in nodes.
		int stopper_ml=0;
		
		while (true) {
					
			stopper_ml++;
			
			int random_mate=nodes[degree_list_in[irand(degree_list_in.size()-1)]];
			while (random_mate==a || random_mate==b)
				random_mate=nodes[degree_list_in[irand(degree_list_in.size()-1)]];
			
			if(Ein[a].find(random_mate)==Ein[a].end()) {
				
				deque <int> not_common;
				for (set<int>::iterator it_est=Eout[random_mate].begin(); it_est!=Eout[random_mate].end(); it_est++)
					if ((b!=(*it_est)) && (Eout[b].find(*it_est)==Eout[b].end()) && (binary_search(nodes.begin(), nodes.end(), *it_est)))
						not_common.push_back(*it_est);
				
				if(not_common.size()>0) {
				
					int node_h=not_common[irand(not_common.size()-1)];
					
					
					
					Eout[random_mate].insert(a);
					Eout[random_mate].erase(node_h);
					
					Ein[node_h].erase(random_mate);
					Ein[node_h].insert(b);
					
					Eout[b].insert(node_h);
					Ein[a].insert(random_mate);
					
					break;

				
			
				}
			
			}
			
			if(stopper_ml==2*Ein.size()) {
	
				cout<<"sorry, I need to change the degree distribution a little bit (one less link)"<<endl;
				break;
	
			}
			
			
			
		}
	
	
	}
	
	

	
	
	return 0;

}





int build_subgraphs(deque<set<int> > & Ein, deque<set<int> > & Eout, const deque<deque<int> > & member_matrix, deque<deque<int> > & member_list, deque<deque<int> > & link_list_in, deque<deque<int> > & link_list_out, 
	const deque<int> & internal_degree_seq_in, const deque<int> & degree_seq_in, const deque<int> & internal_degree_seq_out, const deque<int> & degree_seq_out, const bool excess, const bool defect) {
	
	
	
	Ein.clear();
	Eout.clear();
	member_list.clear();
	link_list_in.clear();
	link_list_out.clear();
	
	int num_nodes=degree_seq_in.size();
	
	
	
	{
		
		deque<int> first;
		for (int i=0; i<num_nodes; i++)
			member_list.push_back(first);
	
	}
	
	
	
	for (int i=0; i<member_matrix.size(); i++)
		for (int j=0; j<member_matrix[i].size(); j++)
			member_list[member_matrix[i][j]].push_back(i);
	
		
	for (int i=0; i<member_list.size(); i++) {
		
		deque<int> liin;
		deque<int> liout;

		
		for (int j=0; j<member_list[i].size(); j++) {
			
			compute_internal_degree_per_node(internal_degree_seq_in[i], member_list[i].size(), liin);
			liin.push_back(degree_seq_in[i] - internal_degree_seq_in[i]);
			compute_internal_degree_per_node(internal_degree_seq_out[i], member_list[i].size(), liout);
			liout.push_back(degree_seq_out[i] - internal_degree_seq_out[i]);

		
		}
		
		link_list_in.push_back(liin);
		link_list_out.push_back(liout);
		
	}
	
	
	/*
	cout<<"link list in out ************************"<<endl;
	printm(link_list_in);
	printm(link_list_out);
	cout<<"link list in out ************************"<<endl;
	*/
	
	// ------------------------ this is done to check if the sums of the internal degrees (in and out) are equal. if not, the program will change it in such a way to assure that. 
	
	
			
	for (int i=0; i<member_matrix.size(); i++) {
	
		
		int internal_cluster_in=0;
		int internal_cluster_out=0;
		
		
		for (int j=0; j<member_matrix[i].size(); j++) {
			
			int right_index= lower_bound(member_list[member_matrix[i][j]].begin(), member_list[member_matrix[i][j]].end(), i) - member_list[member_matrix[i][j]].begin();
			internal_cluster_in+=link_list_in[member_matrix[i][j]][right_index];
			internal_cluster_out+=link_list_out[member_matrix[i][j]][right_index];
			
		
		}
		
		//cout<<"internal_cluster difference "<<internal_cluster_in - internal_cluster_out<<" for nodes: "<<member_matrix[i].size()<<endl;
		
		
		int initial_diff= abs(internal_cluster_in - internal_cluster_out);
		for(int diffloop=0; diffloop<3*initial_diff; diffloop++) {
			
			
			if((internal_cluster_in - internal_cluster_out)==0)
				break;
			
						
			
				
			// if this does not work in a reasonable time the degree sequence will be changed
				
			for (int j=0; j<member_matrix[i].size(); j++) {		
				
				
				
				int random_mate=member_matrix[i][irand(member_matrix[i].size()-1)];
				int right_index= lower_bound(member_list[random_mate].begin(), member_list[random_mate].end(), i) - member_list[random_mate].begin();
				
				if(internal_cluster_in>internal_cluster_out) {
					
					if ((link_list_out[random_mate][right_index]<member_matrix[i].size()-1) && (link_list_out[random_mate][link_list_out[random_mate].size()-1] > 0 )) {
					
						link_list_out[random_mate][right_index]++;
						link_list_out[random_mate][link_list_out[random_mate].size()-1]--;
						internal_cluster_out++;
						
						break;
					}
				}
				
				else if (link_list_out[random_mate][right_index] > 0) {
					
					link_list_out[random_mate][right_index]--;
					link_list_out[random_mate][link_list_out[random_mate].size()-1]++;
					internal_cluster_out--;

					break;
				
				}
			
			}			
					
		}


		//cout<<"internal_cluster difference after "<<internal_cluster_in - internal_cluster_out<<endl;
		
		for(int diffloop=0; diffloop<3*initial_diff; diffloop++) {
			
			
			if((internal_cluster_in - internal_cluster_out)==0)
				break;
			
						
			
				
			// if this does not work in a reasonable time the degree sequence will be changed
				
			for (int j=0; j<member_matrix[i].size(); j++) {		
				
				
				
				int random_mate=member_matrix[i][irand(member_matrix[i].size()-1)];
				int right_index= lower_bound(member_list[random_mate].begin(), member_list[random_mate].end(), i) - member_list[random_mate].begin();
				
				if(internal_cluster_in>internal_cluster_out) {
					
					if ((link_list_out[random_mate][right_index]<member_matrix[i].size()-1)) {
					
						link_list_out[random_mate][right_index]++;
						internal_cluster_out++;
						
						break;
					}
				}
				
				else {
					
					link_list_out[random_mate][right_index]--;
					internal_cluster_out--;

					break;
				
				}
			
			}			
					
		}
		
		
		
		//cout<<"internal_cluster difference after after "<<internal_cluster_in - internal_cluster_out<<endl;
	
	}
	
	
	// ------------------------ this is done to check if the sums of the internal degrees (in and out) are equal. if not, the program will change it in such a way to assure that. 
	
	
		
	{
	
		set<int> first;
		for(int i=0; i<num_nodes; i++) {
			Ein.push_back(first);
			Eout.push_back(first);
			
		}
	
	}
	
	for (int i=0; i<member_matrix.size(); i++) {
		
		
		deque<int> internal_degree_in;
		deque<int> internal_degree_out;

		for (int j=0; j<member_matrix[i].size(); j++) {
		
			int right_index= lower_bound(member_list[member_matrix[i][j]].begin(), member_list[member_matrix[i][j]].end(), i) - member_list[member_matrix[i][j]].begin();
			internal_degree_in.push_back(link_list_in[member_matrix[i][j]][right_index]);
			internal_degree_out.push_back(link_list_out[member_matrix[i][j]][right_index]);

		}		
		

		
		if(build_subgraph(Ein, Eout, member_matrix[i], internal_degree_in, internal_degree_out)==-1)
			return -1;
	
	
	}




	return 0;
	
}


bool they_are_mate(int a, int b, const deque<deque<int> > & member_list) {


	for(int i=0; i<member_list[a].size(); i++) {
		
		if(binary_search(member_list[b].begin(), member_list[b].end(), member_list[a][i]))
			return true;
	
	}

	return false;

}


int compute_var_mate(deque<set<int> > & en_in,  const deque<deque<int> > & member_list) {



	int var_mate=0;
	for(int i=0; i<en_in.size(); i++) for(set<int>::iterator itss= en_in[i].begin(); itss!=en_in[i].end(); itss++) if(they_are_mate(i, *itss, member_list)) {
		var_mate++;
	}



	return var_mate;
}

int connect_all_the_parts(deque<set<int> > & Ein, deque<set<int> > & Eout, const deque<deque<int> > & member_list, const deque<deque<int> > & link_list_in, const deque<deque<int> > & link_list_out) {

	
	deque<int> d_in;
	for(int i=0; i<link_list_in.size(); i++)
		d_in.push_back(link_list_in[i][link_list_in[i].size()-1]);
	
	
	deque<int> d_out;
	for(int i=0; i<link_list_out.size(); i++)
		d_out.push_back(link_list_out[i][link_list_out[i].size()-1]);
		
	/*
	prints(d_in);
	prints(d_out);
	*/

	
	deque<set<int> > en_in;			// this is the Ein of the subgraph
	deque<set<int> > en_out;		// this is the Eout of the subgraph
	
	
	{
		set<int> first;
		for(int i=0; i<member_list.size(); i++) {
			en_in.push_back(first);
			en_out.push_back(first);
		}
	}
	
	
	
	multimap <int, int> degree_node_out;
	deque<pair<int, int> > degree_node_in;
	
	for(int i=0; i<d_out.size(); i++)
		degree_node_out.insert(make_pair(d_out[i], i));
	
	deque<int> fakes;
	for(int i=0; i<d_in.size(); i++)
		fakes.push_back(i);
	
	shuffle_s(fakes);
	
	//prints(fakes);
	deque<int> antifakes(fakes.size());
	for(int i=0; i<d_in.size(); i++)
		antifakes[fakes[i]]=i;
	
	
	
	for(int i=0; i<d_in.size(); i++)
		degree_node_in.push_back(make_pair(d_in[i], fakes[i]));
	
	//printm(degree_node_in);

	sort(degree_node_in.begin(), degree_node_in.end());
	
	//printm(degree_node_in);

	for(int i=0; i<d_in.size(); i++)
		degree_node_in[i].second=antifakes[degree_node_in[i].second];
	
	/*
	prints(d_in);
	printm(degree_node_in);
	*/
	
	deque<pair<int, int> >::iterator itlast = degree_node_in.end();
	
	deque<int> self_loop;
	
	
	//cout<<"difference in connect_all_parts "<<deque_int_sum(d_in) - deque_int_sum(d_out)<<endl;
	while (itlast != degree_node_in.begin()) {
		
		itlast--;
		
		
		multimap <int, int>::iterator itit= degree_node_out.end();
		deque <multimap<int, int>::iterator> erasenda;
		
		
		for (int i=0; i<itlast->first; i++) {
			
			if(itit!=degree_node_out.begin()) {
				
				itit--;
				
				if (itit->second!=itlast->second) {
					
					en_in[itlast->second].insert(itit->second);
					en_out[itit->second].insert(itlast->second);
				
				}
				else
					self_loop.push_back(itlast->second);
				

				erasenda.push_back(itit);
				
			}
			
			else
				break;
		
		}
		
		
		for (int i=0; i<erasenda.size(); i++) {
			
			
			if(erasenda[i]->first>1)
				degree_node_out.insert(make_pair(erasenda[i]->first - 1, erasenda[i]->second));
	
			
			degree_node_out.erase(erasenda[i]);

			
		
		}

		
		
	}
	
	//cout<<"left "<<degree_node_out.size()<<endl;
	//cout<<"self loops "<<self_loop.size()<<endl;
    
    
    deque<int> degree_list_in;
    for(int kk=0; kk<d_in.size(); kk++)
        for(int k2=0; k2<d_in[kk]; k2++)
            degree_list_in.push_back(kk); 

    
	for(int i=0; i<self_loop.size(); i++) {
	
		int node=self_loop[i];

		int stopper=d_in.size()*d_in.size();
		int stop=0;
		
		//cout<<"node "<<node<<endl;
		
		bool breaker=false;

		while (stop++ < stopper) {
		
			//cout<<stop<<" "<<node<<endl;
			
			while(true) {
				
				
				int random_mate=degree_list_in[irand(degree_list_in.size()-1)];
				if(random_mate==node || en_in[node].find(random_mate)!=en_in[node].end())
					break;
				
				deque <int> not_common;
				for (set<int>::iterator it_est=en_out[random_mate].begin(); it_est!=en_out[random_mate].end(); it_est++)
					if (en_out[node].find(*it_est)==en_out[node].end())
						not_common.push_back(*it_est);
					
				if (not_common.empty())
					break;
				

				
				int random_neigh=not_common[irand(not_common.size()-1)];

				
				en_out[node].insert(random_neigh);
				en_in[node].insert(random_mate);
				
				
				en_in[random_neigh].insert(node);
				en_in[random_neigh].erase(random_mate);
				
				en_out[random_mate].insert(node);
				en_out[random_mate].erase(random_neigh);
				
				breaker=true;
				break;
			
			
			}
			
			if(breaker)
				break;
			
		
		}
		
		
		


	}

	
	// this is to randomize the subgraph -------------------------------------------------------------------

	
	for(int run=0; run<10; run++) for(int node_a=0; node_a<d_in.size(); node_a++) for(int krm=0; krm<en_out[node_a].size(); krm++) {
				
				
		int random_mate=degree_list_in[irand(degree_list_in.size()-1)];
		while (random_mate==node_a)
			random_mate=degree_list_in[irand(degree_list_in.size()-1)];
				
		
		if (en_out[node_a].find(random_mate)==en_out[node_a].end()) {
			
			deque <int> external_nodes;
			for (set<int>::iterator it_est=en_out[node_a].begin(); it_est!=en_out[node_a].end(); it_est++)
				external_nodes.push_back(*it_est);
						
										
			int	old_node=external_nodes[irand(external_nodes.size()-1)];
					
			
			deque <int> not_common;
			for (set<int>::iterator it_est=en_in[random_mate].begin(); it_est!=en_in[random_mate].end(); it_est++)
				if ((old_node!=(*it_est)) && (en_in[old_node].find(*it_est)==en_in[old_node].end()))
					not_common.push_back(*it_est);
					
			
			if (not_common.empty())
				break;
				
			int node_h=not_common[irand(not_common.size()-1)];
			
			
			en_out[node_a].insert(random_mate);
			en_out[node_a].erase(old_node);
			
			en_in[old_node].insert(node_h);
			en_in[old_node].erase(node_a);
			
			en_in[random_mate].insert(node_a);
			en_in[random_mate].erase(node_h);
			
			en_out[node_h].erase(random_mate);
			en_out[node_h].insert(old_node);
			

		}
	
	
	}

	
	
	// now there is a rewiring process to avoid "mate nodes" (nodes with al least one membership in common) to link each other
	
	int var_mate= compute_var_mate(en_in, member_list);	
	//cout<<"var mate = "<<var_mate<<endl;
	
	int stopper_mate=0;
	int mate_trooper=10;
	
	while(var_mate>0) {
	
		
		//cout<<"var mate = "<<var_mate<<endl;

		
		int best_var_mate=var_mate;
	
		// ************************************************  rewiring
		
		
		for(int a=0; a<d_in.size(); a++) for(set<int>::iterator its= en_in[a].begin(); its!=en_in[a].end(); its++) if(they_are_mate(a, *its, member_list)) {
				
			
			
			int b=*its;
			int stopper_m=0;
			
			while (true) {
						
				stopper_m++;
				
				int random_mate = degree_list_in[irand(degree_list_in.size()-1)];
				while (random_mate==a || random_mate==b)
					random_mate = degree_list_in[irand(degree_list_in.size()-1)];
				
				
				if(!(they_are_mate(a, random_mate, member_list)) && (en_in[a].find(random_mate)==en_in[a].end())) {
					
					deque <int> not_common;
					for (set<int>::iterator it_est=en_out[random_mate].begin(); it_est!=en_out[random_mate].end(); it_est++)
						if ((b!=(*it_est)) && (en_out[b].find(*it_est)==en_out[b].end()))
							not_common.push_back(*it_est);
					
					if(not_common.size()>0) {
					
						int node_h=not_common[irand(not_common.size()-1)];
						
						
						en_out[random_mate].erase(node_h);
						en_out[random_mate].insert(a);
						
						en_in[node_h].erase(random_mate);
						en_in[node_h].insert(b);
						
						en_out[b].erase(a);
						en_out[b].insert(node_h);
						
						en_in[a].insert(random_mate);
						en_in[a].erase(b);
						
											
						
						if(!they_are_mate(b, node_h, member_list))
							var_mate--;
						
						
						if(they_are_mate(random_mate, node_h, member_list))
							var_mate--;
						
						break;

					
				
					}
				
				}
				
				if(stopper_m==en_in[a].size())
					break;
				
				
				
			}
				
			
			break;		// this break is done because if you erased some link you have to stop this loop (en[i] changed)
	
	
		}

		// ************************************************  rewiring
		
		
				

		if(var_mate==best_var_mate) {
			
			stopper_mate++;
			
			if(stopper_mate==mate_trooper)
				break;

		}
		else
			stopper_mate=0;
		
		
		
		//cout<<"var mate = "<<var_mate<<endl;

	
	}
	
	
	
	//cout<<"var mate = "<<var_mate<<endl;

	for (int i=0; i<en_in.size(); i++) {
		
		for(set<int>::iterator its=en_in[i].begin(); its!=en_in[i].end(); its++) {
		
			Ein[i].insert(*its);
			Eout[*its].insert(i);
			
		
		}
	
	
	}
	
	
	
	return 0;

}

int internal_kin(deque<set<int> > & Ein, const deque<deque<int> > & member_list, int i) {
	
	int var_mate2=0;
	for(set<int>::iterator itss= Ein[i].begin(); itss!=Ein[i].end(); itss++) if(they_are_mate(i, *itss, member_list)) 
		var_mate2++;	

	return var_mate2;
	
}




int internal_kin_only_one(set<int> & Ein, const deque<int> & member_matrix_j) {		// return the overlap between Ein and member_matrix_j
	
	int var_mate2=0;
	
	for(set<int>::iterator itss= Ein.begin(); itss!=Ein.end(); itss++) {
	
		if(binary_search(member_matrix_j.begin(), member_matrix_j.end(), *itss))
			var_mate2++;
	
	}
	
	return var_mate2;
	
}

int erase_links(deque<set<int> > & Ein, deque<set<int> > & Eout, const deque<deque<int> > & member_list, const bool excess, const bool defect, const double mixing_parameter) {

	
	int num_nodes= member_list.size();
	
	int eras_add_times=0;
	
	if (excess) {
		
		for (int i=0; i<num_nodes; i++) {
			
			
			while ( (Ein[i].size()>1) &&  double(internal_kin(Ein, member_list, i))/Ein[i].size() < 1 - mixing_parameter) {
			
			//---------------------------------------------------------------------------------
				
				
				cout<<"degree sequence changed to respect the option -sup ... "<<++eras_add_times<<endl;
				
				deque<int> deqar;
				for (set<int>::iterator it_est=Ein[i].begin(); it_est!=Ein[i].end(); it_est++)
					if (!they_are_mate(i, *it_est, member_list))
						deqar.push_back(*it_est);
				
				
				if(deqar.size()==Ein[i].size()) {	// this shouldn't happen...
				
					cerr<<"sorry, something went wrong: there is a node which does not respect the constraints. (option -sup)"<<endl;
					return -1;
				
				}
				
				int random_mate=deqar[irand(deqar.size()-1)];
				
				Ein[i].erase(random_mate);
				Eout[random_mate].erase(i);
				
		
			}
		}
	
	}
	
	
	
	if (defect) {
			
		for (int i=0; i<num_nodes; i++)
			while ( (Ein[i].size()<Ein.size()) &&  double(internal_kin(Ein, member_list, i))/Ein[i].size() > 1 - mixing_parameter) {
				
				//---------------------------------------------------------------------------------
					
				
				cout<<"degree sequence changed to respect the option -inf ... "<<++eras_add_times<<endl;


				int stopper_here=num_nodes;
				int stopper_=0;
				
				int random_mate=irand(num_nodes-1);
				while ( (    (they_are_mate(i, random_mate, member_list)) || Ein[i].find(random_mate)!=Ein[i].end())      &&      (stopper_<stopper_here) ) {
					
					random_mate=irand(num_nodes-1);
					stopper_++;
				
				
				}
				
				if(stopper_==stopper_here) {	// this shouldn't happen...
				
					cerr<<"sorry, something went wrong: there is a node which does not respect the constraints. (option -inf)"<<endl;
					return -1;
				
				}
				
				
				
				Ein[i].insert(random_mate);
				Eout[random_mate].insert(i);
				
								
		
			}
			
		
	}

	//------------------------------------ Erasing links   ------------------------------------------------------

	


	return 0;
	
}


int print_network(deque<set<int> > & Ein, deque<set<int> > & Eout, const deque<deque<int> > & member_list, const deque<deque<int> > & member_matrix, deque<int> & num_seq) {

	
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

		set<int>::iterator itb=Eout[u].begin();
	
		while (itb!=Eout[u].end())
			out1<<u+1<<"\t"<<*(itb++)+1<<endl;
		

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

	


	cout<<endl<<endl;

	return 0;

}


	



int benchmark(bool excess, bool defect, int num_nodes, double  average_k, int  max_degree, double  tau, double  tau2, 
	double  mixing_parameter, int  overlapping_nodes, int  overlap_membership, int  nmin, int  nmax, bool  fixed_range) {	

	
	
	
	
	
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
	
	
	cout<<"recording network..."<<endl;	
	print_network(Ein, Eout, member_list, member_matrix, num_seq);

	//printmcs(Ein, Eout);
	//printmcs(Eout, Ein);
	
		
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
	
	benchmark(p.excess, p.defect, p.num_nodes, p.average_k, p.max_degree, p.tau, p.tau2, p.mixing_parameter, p.overlapping_nodes, p.overlap_membership, p.nmin, p.nmax, p.fixed_range);	
		
	return 0;
	
}


