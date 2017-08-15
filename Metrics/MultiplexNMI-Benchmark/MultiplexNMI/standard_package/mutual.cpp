	


#if !defined(MUTUAL_INCLUDED)
#define MUTUAL_INCLUDED	




int overlap_grouping(deque<deque<int> > ten, int unique) {		//hrepiguhpueh


	set <int> conta;
	int all=0;
	
			
	for (int i=0; i<int(ten.size()); i++) {
				
		for (int j=0; j<int(ten[i].size()); j++)
			conta.insert(ten[i][j]);
		
		all+=ten[i].size();
	
	}


	unique=conta.size();
	
	int overlap=all - unique;
	
	


	return overlap;

}
 


double mutual (deque<deque<int> > en, deque<deque<int> > ten) {
		
		
		// en e ten are two partitions of integer numbers
		int dim;
		
		
		{
			set <int> conta;
			//set <int> ten_;
			//set <int> en_;
			
			for (int i=0; i<int(ten.size()); i++) {
				sort(ten[i].begin(), ten[i].end());
				for (int j=0; j<int(ten[i].size()); j++) {
					conta.insert(ten[i][j]);
					//ten_.insert(ten[i][j]);
				}
			}
			
			for (int i=0; i<int(en.size()); i++) {
				sort(en[i].begin(), en[i].end());
				for (int j=0; j<int(en[i].size()); j++) {
					conta.insert(en[i][j]);
					//en_.insert(en[i][j]);

				}
			}
			
			
			
			
			dim=conta.size();
			
			/*
			for (set<int>::iterator its=conta.begin(); its!=conta.end(); its++) {
				
				if(ten_.find(*its)==ten_.end()) {

					deque <int> first;
					first.push_back(*its);
					ten.push_back(first);
				}
				
				if(en_.find(*its)==en_.end()) {

					deque <int> first;
					first.push_back(*its);
					en.push_back(first);
				}
				
				
			}
			*/
			
			
		}
		
		
		//cout<<"dim:\t"<<dim<<endl;
		
		
		deque<deque<double> > N;
		deque <double> first;
		first.assign(en.size(), 0);
		for (int i=0; i<int(ten.size()); i++)
			N.push_back(first);
		
		deque <int> s (dim);
		for (int i=0; i<int(ten.size()); i++)
			for (int j=0; j<int(en.size()); j++)
				N[i][j]=set_intersection(ten[i].begin(),ten[i].end(),en[j].begin(),en[j].end(),s.begin())-s.begin();
		
		//printv(N);
		
		
		/*
		cout<<"one:"<<endl;
		printm(ten);
	
		
		cout<<"two:"<<endl;
		printm(en);
		
				
		cout<<"confusion matrix"<<endl;
		printm(N, cout);
		*/
		
		
		//cout<<"confusion matrix"<<endl;
		//printm(N, cout);

		
		
		
		
		
		
		deque <double> NR;
		NR.assign(ten.size(), 0);
		deque <double> NC;
		NC.assign(en.size(), 0);
		double NTOT=dim;
			
		for (int i=0; i<int(ten.size()); i++)
			for (int j=0; j<int(en.size()); j++) {
				NR[i]+=N[i][j];
				NC[j]+=N[i][j];
			}
			
		
			
		double IN=0;
		double ID1=0;
		double ID2=0;
			
		for (int i=0; i<int(ten.size()); i++)
			for (int j=0; j<int(en.size()); j++)
				if (N[i][j]!=0)
					IN+=N[i][j]*log(N[i][j]*NTOT/(NR[i]*NC[j]));
								
		IN=-2.*IN;
			
			
		for (int i=0; i<int(ten.size()); i++)
			if (NR[i]!=0)
				ID1+=NR[i]*log(NR[i]/(NTOT));
					
			
					
		for (int j=0; j<int(en.size()); j++)
			if (NC[j]!=0)
				ID2+=NC[j]*log(NC[j]/(NTOT));
					



		double I= IN/(ID1+ID2);

		if ((ID1+ID2)==0)
			I=-2;
			
		
		return I;

}


double H(double a) {


	if(a<=0)
		return 0;
	else
		return (- a * log(a));

}


double H(deque <double> &p) {
	
	double h=0;
	for (deque<double>::iterator it=p.begin(); it!=p.end(); it++) if (*it!=0)
		h+=(*it)*log(*it);
	
	return (-h);


}


double H_x_given_y(deque<deque<int> > &en, deque<deque<int> > &ten, int dim) {

		// you know y and you want to find x according to a certain index labelling.
		// so, for each x you look for the best y.
		
		double H_x_y=0;
		double H2=0;
				
		for (int j=0; j<int(en.size()); j++) {
			
			
			deque <double> p;
			double I2=double(en[j].size());
			double O2=(dim-I2);
			p.push_back(I2/dim);
			p.push_back(O2/dim);
			double H2_=H(p);
			p.clear();
			
				
			H2+=H2_;
			
			double diff=H2_;
			
			for (int i=0; i<int(ten.size()); i++) {
				
							
				
				double I1=double(ten[i].size());
				double O1=(dim-I1);
				
				//cout<<"I1 "<<I1<<" O1 "<<O1<<endl;

				p.push_back(I1/dim);
				p.push_back(O1/dim);
				double H1_=H(p);
				p.clear();
				
				//prints(ten[i]);
				//cout<<"H1_: "<<H1_<<"\t";


				
				deque <int> s (dim);
				double I1_I2=set_intersection(ten[i].begin(), ten[i].end(), en[j].begin(), en[j].end(),s.begin())-s.begin();	// common
				double I1_02=set_difference(ten[i].begin(), ten[i].end(), en[j].begin(), en[j].end(),s.begin())-s.begin();	
				double O1_I2=set_difference(en[j].begin(), en[j].end(), ten[i].begin(), ten[i].end(),s.begin())-s.begin();	
				double O1_02=dim - I1_I2 - I1_02 - O1_I2;
				
				
				p.push_back(I1_I2/dim);
				p.push_back(O1_02/dim);
				
				double H12_positive=H(p);
				
				p.clear();
				p.push_back(I1_02/dim);
				p.push_back(O1_I2/dim);
				
				
				double H12_negative=H(p);

				double H12_=H12_negative+H12_positive;
				
				p.clear();
				
				if (H12_negative>H12_positive) {
					
					H12_=H1_+H2_;
					//cout<<"the negative part is bigger"<<endl;
					//prints(en[j]);
					//prints(ten[i]);
				}
				
				
				/*
				
				cout<<"worst case "<<H1_+H2_<<"\ttotal "<<H12_negative+H12_positive<<"\tnegative part "<<H12_negative<<"\tpositive part "<<H12_positive<<endl;
				prints(en[j]);
				prints(ten[i]);
				
				*/
				
				//cout<<"entropies "<<H1_<<" "<<H2_<<" "<<H12_<<endl;
				
				if ((H12_-H1_)<diff) {
					diff=(H12_-H1_);
				}
				
				
				
			}
			
			
			//H_x_y+=diff;
			if (H2_==0)
				H_x_y+=1;
			else
				H_x_y+=(diff/H2_);
		
		}
		
		//if (H2==0)
		//	return 1;
		
		return (H_x_y/(en.size()));

}


double mutual2(deque<deque<int> > en, deque<deque<int> > ten) {
		
		
		
		if(en.size()==0 || ten.size()==0)
			return 0;
		
		
		
		// en e ten are two partitions of integer numbers
		int dim;
		
		
		//printm(ten);
		//printm(en);


		{	
			map <int, int> all;		// node, index 
			//set <int> ten_;
			//set <int> en_;
			
			for (int i=0; i<int(ten.size()); i++) {
				for (int j=0; j<int(ten[i].size()); j++) {
					all.insert(make_pair(ten[i][j], all.size()));
					//ten_.insert(ten[i][j]);
				}
			}
			
			for (int i=0; i<int(en.size()); i++) {
				for (int j=0; j<int(en[i].size()); j++) {
					all.insert(make_pair(en[i][j], all.size()));
					//en_.insert(en[i][j]);

				}
			}
			
			
			
			
			dim=all.size();
			
			
			/*
			for (map<int, int>::iterator its=all.begin(); its!=all.end(); its++) {
				
				if(ten_.find(its->first)==ten_.end()) {

					deque <int> first;
					first.push_back(its->first);
					ten.push_back(first);
				}
				
				if(en_.find(its->first)==en_.end()) {

					deque <int> first;
					first.push_back(its->first);
					en.push_back(first);
				}
				
				
			}
			*/
			
			
			for (int i=0; i<int(ten.size()); i++) {
				for (int j=0; j<int(ten[i].size()); j++)
					ten[i][j]=all[ten[i][j]];
				
				sort(ten[i].begin(), ten[i].end());
			}
			
			for (int i=0; i<int(en.size()); i++) {
				for (int j=0; j<int(en[i].size()); j++)
					en[i][j]=all[en[i][j]];
				
				sort(en[i].begin(), en[i].end());
			}

			
			
			
			
		}
		
		
		
		return (0.5*( 2. - H_x_given_y(ten, en, dim) - H_x_given_y(en, ten, dim)));
		
}

double H_x_given_y3(deque<deque<int> > &en, deque<deque<int> > &ten, int dim) {
	
	// you know y and you want to find x according to a certain index labelling.
	// so, for each x you look for the best y.
	
	
	deque<deque<int> > mems;
	
	deque<int> first;
	for(int i=0; i<dim; i++)
		mems.push_back(first);
	
	for (int ii=0; ii<int(ten.size()); ii++)
		for(int i=0; i<int(ten[ii].size()); i++)
			mems[ten[ii][i]].push_back(ii);

	
	
	double H_x_y=0;
				
	for (int k=0; k<int(en.size()); k++) {
		
		
		
		deque<int> & c = en[k];
		
		
		deque <double> p;
		double I2=double(c.size());
		double O2=(dim-I2);
		p.push_back(I2/dim);
		p.push_back(O2/dim);
		double H2_=H(p);
		p.clear();
		
		
		
		double diff=H2_;
		
		// I need to know all the group with share nodes with en[k]
		
		map<int, int> com_ol;		// it maps the index of the ten into the overlap with en[k]
		
		for(int i=0; i<int(c.size()); i++) {
			
			for(int j=0; j<int(mems[c[i]].size()); j++)
				int_histogram(mems[c[i]][j], com_ol);
			
		}
		
		
				
		for(map<int, int>::iterator itm=com_ol.begin(); itm!=com_ol.end(); itm++) {		
			
			
			
			double I1=double(ten[itm->first].size());
			double O1=(dim-I1);
			
			
			p.push_back(I1/dim);
			p.push_back(O1/dim);
			double H1_=H(p);
			p.clear();
			
			
			
			
			double I1_I2= itm->second;
			double I1_02= ten[itm->first].size() - I1_I2;	
			double O1_I2= c.size() - I1_I2;	
			double O1_02= dim - I1_I2 - I1_02 - O1_I2;
			
			
			p.push_back(I1_I2/dim);
			p.push_back(O1_02/dim);
			
			double H12_positive=H(p);
			
			p.clear();
			p.push_back(I1_02/dim);
			p.push_back(O1_I2/dim);
			
			
			double H12_negative=H(p);
			
			double H12_=H12_negative+H12_positive;
			
			p.clear();
			
			if (H12_negative>H12_positive) {
				
				H12_=H1_+H2_;
				
			}
			
			
			
			
			
			if ((H12_-H1_)<diff) {
				diff=(H12_-H1_);
			}
			
			
			
		}
		
		
		
		if (H2_==0)
			H_x_y+=1;
		else
			H_x_y+=(diff/H2_);
		
	}
	
	
	
	
	
	
	
	return (H_x_y/(en.size()));
	
}




double mutual3(deque<deque<int> > en, deque<deque<int> > ten) {
		
		
		
		if(en.size()==0 || ten.size()==0)
			return 0;
		
		
		
		// en e ten are two partitions of integer numbers
		int dim;
		
		
	
		{	
			map <int, int> all;		// node, index 
			//set <int> ten_;
			//set <int> en_;
			
			for (int i=0; i<int(ten.size()); i++) {
				for (int j=0; j<int(ten[i].size()); j++) {
					all.insert(make_pair(ten[i][j], all.size()));
					//ten_.insert(ten[i][j]);
				}
			}
			
			for (int i=0; i<int(en.size()); i++) {
				for (int j=0; j<int(en[i].size()); j++) {
					all.insert(make_pair(en[i][j], all.size()));

				}
			}
			
			
			
			
			dim=all.size();
			
			
			
			for (int i=0; i<int(ten.size()); i++) {
				for (int j=0; j<int(ten[i].size()); j++)
					ten[i][j]=all[ten[i][j]];
				
				sort(ten[i].begin(), ten[i].end());
			}
			
			for (int i=0; i<int(en.size()); i++) {
				for (int j=0; j<int(en[i].size()); j++)
					en[i][j]=all[en[i][j]];
				
				sort(en[i].begin(), en[i].end());
			}

			
			
			
			
		}
		
			
		
		
		return (0.5*( 2. - H_x_given_y3(ten, en, dim) - H_x_given_y3(en, ten, dim)));
		
}



double c_nodes_jacc(int_matrix & ten, int_matrix & en, int dim) {
	
	
	// this function does a best match based on the jaccard index.
	// note that it should be weighted on the cluster size (I believe)
	
	
	deque<deque<int> > mems;
	
	deque<int> first;
	for(int i=0; i<dim; i++)
		mems.push_back(first);
	
	for (int ii=0; ii<int(ten.size()); ii++)
		for(int i=0; i<int(ten[ii].size()); i++)
			mems[ten[ii][i]].push_back(ii);
	
	double global_overlap=0;
	RANGE_loop(k, en) {
		
		deque<int> & c = en[k];
		
		map<int, int> com_ol;		// it maps the index of the ten into the overlap with en[k]
		
		RANGE_loop(i, c) {
			
			for(int j=0; j<int(mems[c[i]].size()); j++)
				int_histogram(mems[c[i]][j], com_ol);
		}
		
		double max_jac=0;
		for(map<int, int>::iterator itm=com_ol.begin(); itm!=com_ol.end(); itm++) {
			
			set<int> s1;
			set<int> s2;
			
			deque_to_set(c, s1);
			deque_to_set(ten[itm->first], s2);
			
			double jc=jaccard(s1, s2);
			cout<<"jc: "<<jc<<endl;
			max_jac=max(max_jac, jc);
			
		}
		
		global_overlap+=max_jac;
		cout<<"========== "<<global_overlap<<endl;
	}
	
	return global_overlap/en.size();
	
}  


int c_nodes(int_matrix & ten, int_matrix & en, int dim) {
		
	deque<deque<int> > mems;
	
	deque<int> first;
	for(int i=0; i<dim; i++)
		mems.push_back(first);
	
	for (int ii=0; ii<int(ten.size()); ii++)
		for(int i=0; i<int(ten[ii].size()); i++)
			mems[ten[ii][i]].push_back(ii);
	
	int global_overlap=0;
	RANGE_loop(k, en) {
		
		deque<int> & c = en[k];
		
		map<int, int> com_ol;		// it maps the index of the ten into the overlap with en[k]
		
		RANGE_loop(i, c) {
			
			for(int j=0; j<int(mems[c[i]].size()); j++)
				int_histogram(mems[c[i]][j], com_ol);
		}
		
		int overlap=0;
		for(map<int, int>::iterator itm=com_ol.begin(); itm!=com_ol.end(); itm++) {
			if(itm->second>overlap)
				overlap=itm->second;
		}
		
		global_overlap+=overlap;
	}
	
	return global_overlap;

}  

double correctly_classified_nodes(deque<deque<int> > en, deque<deque<int> > ten) {
	
	
	if(en.size()==0 || ten.size()==0)
		return 0;
	// en e ten are two partitions of integer numbers
	int dim;
	
	
	{	
		map <int, int> all;		// node, index 
		//set <int> ten_;
		//set <int> en_;
		
		for (int i=0; i<int(ten.size()); i++) {
			for (int j=0; j<int(ten[i].size()); j++) {
				all.insert(make_pair(ten[i][j], all.size()));
				//ten_.insert(ten[i][j]);
			}
		}
		
		for (int i=0; i<int(en.size()); i++) {
			for (int j=0; j<int(en[i].size()); j++) {
				all.insert(make_pair(en[i][j], all.size()));
				
			}
		}
		
		dim=all.size();
		
		for (int i=0; i<int(ten.size()); i++) {
			for (int j=0; j<int(ten[i].size()); j++)
				ten[i][j]=all[ten[i][j]];
			
			sort(ten[i].begin(), ten[i].end());
		}
		
		for (int i=0; i<int(en.size()); i++) {
			for (int j=0; j<int(en[i].size()); j++)
				en[i][j]=all[en[i][j]];
			
			sort(en[i].begin(), en[i].end());
		}
	}
	
	
	
	int c1= c_nodes(en, ten, dim);
	//cout<<"c1: "<<c1<<endl;
	
	int c2= c_nodes(ten, en, dim);
	//cout<<"c2: "<<c2<<endl;
	
	return double(c1+c2)/dim/2.;
	
}



/*
void H_x_given_y4(deque<deque<int> > &en, deque<deque<int> > &ten, int dim, double & H_X, double & H_X_given_Y) {
	
	// you know y and you want to find x according to a certain index labelling.
	// so, for each x you look for the best y.
	
	
	H_X=0;
	H_X_given_Y=0;
	
	deque<deque<int> > mems;
	
	deque<int> first;
	for(int i=0; i<dim; i++)
		mems.push_back(first);
	
	for (UI ii=0; ii<ten.size(); ii++)
		for(int i=0; i<ten[ii].size(); i++)
			mems[ten[ii][i]].push_back(ii);
	
	
	
	double H_x_y=0;
	double H2=0;
	
	for (UI k=0; k<en.size(); k++) {
		
		
		
		deque<int> & c = en[k];
		
		
		deque <double> p;
		double I2=double(c.size());
		double O2=(dim-I2);
		p.push_back(I2/dim);
		p.push_back(O2/dim);
		double H2_=H(p);
		p.clear();
		
		
		
		double diff=H2_;
		
		// I need to know all the groups which share nodes with en[k]
		
		map<int, int> com_ol;		// it maps the index of the ten into the overlap with en[k]
		
		for(UI i=0; i<c.size(); i++) {
			
			for(int j=0; j<mems[c[i]].size(); j++)
				int_histogram(mems[c[i]][j], com_ol);
			
		}
		
		
		
		for(map<int, int>::iterator itm=com_ol.begin(); itm!=com_ol.end(); itm++) {		
			
			
			
			double I1=double(ten[itm->first].size());
			double O1=(dim-I1);
			
			
			p.push_back(I1/dim);
			p.push_back(O1/dim);
			double H1_=H(p);
			p.clear();
			
			
			
			
			double I1_I2= itm->second;
			double I1_02= ten[itm->first].size() - I1_I2;	
			double O1_I2= c.size() - I1_I2;	
			double O1_02= dim - I1_I2 - I1_02 - O1_I2;
			
			
			p.push_back(I1_I2/dim);
			p.push_back(O1_02/dim);
			
			double H12_positive=H(p);
			
			p.clear();
			p.push_back(I1_02/dim);
			p.push_back(O1_I2/dim);
			
			
			double H12_negative=H(p);
			
			double H12_=H12_negative+H12_positive;
			
			p.clear();
			
			if (H12_negative>H12_positive) {
				
				H12_=H1_+H2_;
				
			}
			
			if ((H12_-H1_)<diff) {
				diff=(H12_-H1_);
			}
			
			
			
		}
		
		
		H_X+=H2_;
		H_X_given_Y+=diff;
		
	}
	
	
}





double mutual4(deque<deque<int> > en, deque<deque<int> > ten) {
	
	
	
	if(en.size()==0 || ten.size()==0)
		return 0;
	
	
	
	// en e ten are two partitions of integer numbers
	int dim;
	

	
	
	
	{	
		map <int, int> all;		// node, index 
		//set <int> ten_;
		//set <int> en_;
		
		for (int i=0; i<int(ten.size()); i++) {
			for (int j=0; j<int(ten[i].size()); j++) {
				all.insert(make_pair(ten[i][j], all.size()));
				//ten_.insert(ten[i][j]);
			}
		}
		
		for (int i=0; i<int(en.size()); i++) {
			for (int j=0; j<int(en[i].size()); j++) {
				all.insert(make_pair(en[i][j], all.size()));
				
			}
		}
		
		
		
		
		dim=all.size();
		
		
		
		for (int i=0; i<int(ten.size()); i++) {
			for (int j=0; j<int(ten[i].size()); j++)
				ten[i][j]=all[ten[i][j]];
			
			sort(ten[i].begin(), ten[i].end());
		}
		
		for (int i=0; i<int(en.size()); i++) {
			for (int j=0; j<int(en[i].size()); j++)
				en[i][j]=all[en[i][j]];
			
			sort(en[i].begin(), en[i].end());
		}
		
		
		
		
		
	}
	
	
	
	
	
	double H_X, H_Y, H_X_given_Y, H_Y_given_X;
	H_x_given_y4(ten, en, dim, H_X, H_X_given_Y);
	H_x_given_y4(en, ten, dim, H_Y, H_Y_given_X);
	
	
	return (0.5*( H_X - H_X_given_Y + H_Y - H_Y_given_X))/max(H_X, H_Y);
	
}


*/



int new_labels(deque<deque<int> > & a, map<int, int> & Alabels) {
	
	
	
	for(UI i=0; i<a.size(); i++)
		for(UI j=0; j<a[i].size(); j++)
			Alabels.insert(make_pair(a[i][j], Alabels.size()));
	
	for(UI i=0; i<a.size(); i++)
		for(UI j=0; j<a[i].size(); j++)
			a[i][j]=Alabels[a[i][j]];
	
	
	return 0;
	
}



int old_labels(deque<deque<int> > & a, map<int, int> & Alabels) {
	
	
	deque<int> old_labels(Alabels.size());
	for(map<int, int>::iterator itm=Alabels.begin(); itm!=Alabels.end(); itm++)
		old_labels[itm->second]=itm->first;	
	for(UI i=0; i<a.size(); i++)
		for(UI j=0; j<a[i].size(); j++)
			a[i][j]=old_labels[a[i][j]];
	
	
	return 0;
	
	
}


int set_membership(deque<deque<int> > & a, deque<vector<int> > & mem, int dim) {
	
	
	vector<int> first;
	first.reserve(5);
	for(int i=0; i<dim; i++)
		mem.push_back(first);
	
	
	for(UI i=0; i<a.size(); i++) {
		
		
		int new_name=i;
		deque<int> & c=a[i];
		for(UI j=0; j<c.size(); j++)
			mem[c[j]].push_back(new_name);
		
	}
	
	
	return 0;
	
	
	
}




int compute_relations(deque<deque<int> >  a, deque<deque<int> >  b) {
	
	
	
	map<int, int> Al;
	new_labels(a, Al);
	new_labels(b, Al);
	
	
	deque<vector<int> > membershipA;
	deque<vector<int> > membershipB;
	int dim=Al.size();
	
	set_membership(a, membershipA, dim);
	set_membership(b, membershipB, dim);
	
	
	
	
	for(UI ii=0; ii<a.size(); ii++) {
		
		
		
		deque<int> & c= a[ii];
		
		map<int, int> com_ol;		
		
		for(UI i=0; i<c.size(); i++) {
			
			for(UI j=0; j<membershipB[c[i]].size(); j++)
				int_histogram(membershipB[c[i]][j], com_ol);
			
		}
		
		
		
		for(map<int, int>::iterator itm=com_ol.begin(); itm!=com_ol.end(); itm++) {
			
			//cout<<"module "<<ii<<" size a "<<c.size()<<" size b "<<b[itm->first].size()<<" overlap "<<itm->second<<endl;
			
			double ov1=double(itm->second)/ c.size();
			double ov2=double(itm->second)/ b[itm->first].size();
			cout<<ii<<" "<<itm->first<<" "<<ov1<<" "<<ov2<<" "<<c.size()<<" "<<b[itm->first].size();
			if(ov1==1 && ov2==1)
				cout<<" ok"<<endl;
			
			if(ov1<1 && ov2==1)
				cout<<" a big"<<endl;
			
			if(ov1==1 && ov2<1)
				cout<<" b big"<<endl;
			
			if(ov1<1 && ov2<1)
				cout<<" min "<<min(ov1, ov2)<<endl;
			
		}
		
		
		
		
	}
	
	
	
	
	
	
	
	return 0;
	
}



double best_match(deque<deque<int> >  a, deque<deque<int> >  b, string outfile, bool inclusion=false) {
		
	// a selects a best module in b
	// outfile reports three indices "i j o":
	// i a module of a best matched with j. o is their jaccard index
	
	/*
	 The equation for the best match is pretty simple.
	 each module c of partition (a) select a module of (b) with best jaccard index c_best
	 the global score is a weighted average on (|c| union |c_best|)
	 */
	
	ofstream pout(outfile.c_str());
	map<int, int> Al;
	new_labels(a, Al);
	new_labels(b, Al);
	
	deque<vector<int> > membershipA;
	deque<vector<int> > membershipB;
	int dim=Al.size();
	
	set_membership(a, membershipA, dim);
	set_membership(b, membershipB, dim);
	
	double global_best_match=0;
	double denominator=0;
	
	ofstream allout("ov_left_right.dat");
	
	for(UI ii=0; ii<a.size(); ii++) {
		
		deque<int> & c= a[ii];
		map<int, int> com_ol;
		for(UI i=0; i<c.size(); i++) {
			for(UI j=0; j<membershipB[c[i]].size(); j++)
				int_histogram(membershipB[c[i]][j], com_ol);
		}

		int c_best_match=0;
		double c_best_jacc=0;
		
		
		for(map<int, int>::iterator itm=com_ol.begin(); itm!=com_ol.end(); itm++) {
			double ov=double(itm->second);
			double unio= c.size() + b[itm->first].size() - ov;
			double Jacc= ov/unio;
			if(inclusion)
				Jacc=ov/(c.size());
			
			allout<<ii<<" "<<10000000+itm->first<<" "<<ov<<" "<<c.size() <<" "<< b[itm->first].size()<<" "<<Jacc<<endl;
			
			if (Jacc > c_best_jacc) {
				c_best_match=itm->first;
				c_best_jacc=Jacc;
			}			
		}
		
		if(c_best_jacc==0)
			cout<<"some groups are not matched at all (left nodes are all missing on the right)"<<endl;
		
		global_best_match+=c_best_jacc*c.size();
		denominator+=c.size();
		pout<<ii<<" "<<c_best_match<<" "<<c_best_jacc<<endl;
	
	}
	
	
	return global_best_match/denominator;
	
}




pair<double, double> mutual_fast(int_matrix a, int_matrix b) {


	map<int, int> Al;
	new_labels(a, Al);
	new_labels(b, Al);
	
	deque<vector<int> > membershipA;
	deque<vector<int> > membershipB;
	int dim=Al.size();
	
	set_membership(a, membershipA, dim);
	set_membership(b, membershipB, dim);
	
	
	DD pxy;
	
	
	for(UI ii=0; ii<a.size(); ii++) {
		
		deque<int> & c= a[ii];
		map<int, int> com_ol;
		for(UI i=0; i<c.size(); i++) {
			for(UI j=0; j<membershipB[c[i]].size(); j++)
				int_histogram(membershipB[c[i]][j], com_ol);
		}
		
		for(map<int, int>::iterator itm=com_ol.begin(); itm!=com_ol.end(); itm++) {
			pxy.push_back(double(itm->second)/dim);
		}
	}
	
	
	double Hxy=H(pxy);
	DD px; 
	DD py;
	RANGE_loop(i, a) px.push_back(double(a[i].size())/dim);
	RANGE_loop(i, b) py.push_back(double(b[i].size())/dim);
	
	double Hx=H(px);
	double Hy=H(py);
	
	double Ixy= Hx + Hy -Hxy;
	//double NMI= Ixy / ( max(Hx,Hy) );
	double NMI= Ixy / ( max(Hx,Hy) );
	double VI= Hx + Hy -2*Ixy;
	
	cout<<"Hx: "<<Hx<<" Hy: "<<Hy<<" Hxy: "<<Hxy<<endl;
	
	return make_pair(NMI, VI);




}


double H_entropy_partition(int_matrix A) {
	
	map<int, int> Al;
	new_labels(A, Al);
	
	deque<vector<int> > membershipA;
	int dim=Al.size();
	
	set_membership(A, membershipA, dim);
	
	//printm(A);
	//printm(membershipA);
	
	double h=0;
	RANGE_loop(i, A) {
		double px=0;
		RANGE_loop(j, A[i]) {
			px+= 1./ membershipA[A[i][j]].size(); 
		}
		px/=dim;
		//cout<<"-> "<<H(px)<<" px: "<<px<<endl; 
		h+=H(px);
	}
	
	return h;
}







#endif

