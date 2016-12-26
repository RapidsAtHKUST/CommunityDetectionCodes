#if !defined(STATIC_static_network_INCLUDED)
#define STATIC_static_network_INCLUDED


#include "./standard_package/standard_include.cpp"
#include "wsarray.h"






class static_network {
	
	
	
	public:
		
		
		static_network(){};
		~static_network();
		

		int draw(string);
		int draw_consecutive(string file_name1, string file_name2);
		int draw_with_weight_probability(string file_name);
		
		
		void print_id(const deque<int> & a, ostream &);
		void print_id(const deque<deque<int> > & , ostream &);
		void print_id(const deque<set<int> > & , ostream &);
		void print_id(const set<int> & , ostream & );
		void deque_id(deque<int> & );
		
		void set_subgraph(deque<int> & group, deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node);
		
			
		int translate(deque<deque<int> > &);
		int translate_anyway(deque<deque<int> > & ten);

		void get_id_label(map <int, int> &);
		int id_of(int a) {return vertices[a]->id_num;};
		
	
		int size() {return dim;};
		double stubs() {return oneM;};
		
		int kin_m (const deque<int> &);
		int kin_m(const set<int> &);
		int ktot_m (const deque<int> &);
		int ktot_m (const set<int> &);
		
	

		void set_graph(map<int, map<int, pair<int, double> > > & A);
		bool set_graph(string file_name);
		void set_graph(deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node, deque<int> & label_rows);
		void clear();
		
		void set_proper_weights();
		
		
		void set_connected_components(deque<deque<int> > & );
		int propagate_distances(deque<int> & new_shell, set<int> & already_gone, deque<pair<int, int > > & distances_node, int shell, deque<double> & ML, int &, int);
		void same_component(int , set<int> &);
		
		
		int set_upper_network(map<int, map<int, pair<int, double> > > & neigh_weight_f, module_collection & Mcoll);
		void print_degree_of_homeless(DI & homel, ostream & outt);
		
		
	protected:



		class  vertex {
				
			public:
				
				vertex(int , int , int);
				~vertex();
				
				void kplus_global_and_quick(deque<int> & a, int & stubs_in, double & strength_in);
				int kplus_m(const deque<int> &);
				double kplus_w(const deque<int> &);
				int kplus_m(const set<int> &);
							
				int id_num;						// id
				double strength;				// sum of the weights
				int stub_number;				// number of stubs
				wsarray* links;					// array with label of neighbor, multiple links, sm of the weights towards it
				deque<double> original_weights;

		};

				
				
		int dim;									// number of nodes
		int oneM;									// number of stubs
		
		deque <vertex*> vertices;
		
		
		

		
		
};









static_network::vertex::vertex(int b, int c, int preall) {
	
	id_num=b;
	links=new wsarray(preall);
	
	
}



static_network::vertex::~vertex() {
	
	delete links;
	links=NULL;
	
	
	
}



void static_network::vertex::kplus_global_and_quick(deque<int> & a, int & stubs_in, double & strength_in) {

	// a must be sorted, otherwise it does not work
	// this function has never been used (so, it should be checked)
	
	stubs_in=0;
	strength_in=0;
	
	if(int(a.size())>stub_number) {
		// in this case there must be a loop on links because a is longer
		
		for (int i=0; i<links->size(); i++) {
			
			if(binary_search(a.begin(), a.end(), links->l[i])) {
				stubs_in+=links->w[i].first;
				strength_in+=links->w[i].second;
			}
		}
	} else {
	
		for (int i=0; i<int(a.size()); i++) {
			
			pair<int, double> A=links->posweightof(a[i]).second;
			stubs_in+=A.first;
			strength_in+=A.second;
		}
	
	}

}




int static_network::vertex::kplus_m(const deque<int> &a) {

	// computes the internal degree of the vertex respect with a
	
	int s=0;
	//double f=0;
	for (int i=0; i<int(a.size()); i++) {
		pair<int, double> A=links->posweightof(a[i]).second;
		s+=A.first;
		//f+=A.second;
	}
	
	
	return s;
	
}



double static_network::vertex::kplus_w(const deque<int> &a) {
	
	// computes the internal degree of the vertex respect with a
	
	double s=0;
	//double f=0;
	for (int i=0; i<int(a.size()); i++) {
		pair<int, double> A=links->posweightof(a[i]).second;
		s+=A.second;
		//f+=A.second;
	}
	
	
	return s;
	
}






int static_network::vertex::kplus_m(const set<int> &a) {
	
	// computes the internal degree of the vertex respect with a (a is supposed to be sorted)
	
	int s=0;
	//double f=0;
	
	for (int i=0; i<links->size(); i++) {
		if(a.find(links->l[i])!=a.end()) {
			
			s+=links->w[i].first;
			//f+=links->w[i].second;
		}
	}
	
	return s;
	
}



static_network::~static_network() {
	
	clear();

}


void static_network::clear() {

	
	for (int i=0; i<int(vertices.size()); i++) {
		
		delete vertices[i];
		vertices[i]=NULL;
	
	}
	
	vertices.clear();
	dim=0;
	oneM=0;
	
	
	

}





void static_network::set_graph(map<int, map<int, pair<int, double> > > & A) {
	
	// this maps the id into the usual stuff neighbors-weights
	
	
	
	deque<deque<int> > link_per_node;
	deque<deque<pair<int, double> > > weights_per_node;
	deque<int> label_rows;
	
	for(map<int, map<int, pair<int, double> > >:: iterator itm= A.begin(); itm!=A.end(); itm++) {
		
		
		label_rows.push_back(itm->first);
		deque<int> n;
		deque<pair<int, double> > w;
		
		
		for(map<int, pair<int, double> >:: iterator itm2=itm->second.begin(); itm2!=itm->second.end(); itm2++) if(itm2->second.first>0) {
			
			n.push_back(itm2->first);
			w.push_back(itm2->second);
		
		}
		
		link_per_node.push_back(n);
		weights_per_node.push_back(w);
	
	
	
	}

	/*
	prints(label_rows);
	printm(link_per_node);
	printm(weights_per_node);*/
	
	
	set_graph(link_per_node, weights_per_node, label_rows);


}



//   all this stuff here should be improved
//   all this stuff here should be improved
//   all this stuff here should be improved

bool static_network::set_graph(string file_name) {
	
	
	clear();
	
	
	char b[file_name.size()+1];
	cast_string_to_char(file_name, b);
	
	
	
	
	map<int, int> newlabels;
	deque<int> link_i;
	
	bool good_file=true;
	
	
	{
	
		int label=0;
		
	
		ifstream inb(b);
		
		string ins;
		
		
		while(getline(inb, ins)) if(ins.size()>0 && ins[0]!='#') {
			
			
			deque<double> ds;
			cast_string_to_doubles(ins, ds);

			if(ds.size()<2) {
				cerr<<"From file "<<file_name<<": string not readable "<<ins<<" "<<endl;
				good_file=false;
				break;
			}
			
			
			else {
				
				
				int innum1=cast_int(ds[0]);
				int innum2=cast_int(ds[1]);
				
				
				map<int, int>::iterator itf=newlabels.find(innum1);
				if (itf==newlabels.end()) {
					newlabels.insert(make_pair(innum1, label++));
					link_i.push_back(1);
				}
				else 
					link_i[itf->second]++;
				
				
				itf=newlabels.find(innum2);
				if (itf==newlabels.end()) {
					newlabels.insert(make_pair(innum2, label++));
					link_i.push_back(1);
				}
				else
					link_i[itf->second]++;

					
				
			
			
			}
			
			
			
		}
	
	}
	
	
	dim=newlabels.size();
	
	for(int i=0; i<dim; i++)
		vertices.push_back(new vertex (0, 0, link_i[i]));
	
	
	for(map<int, int>::iterator itm=newlabels.begin(); itm!=newlabels.end(); itm++)
		vertices[itm->second]->id_num=itm->first;
	
	
	
	if(good_file) {
	
	
		ifstream inb(b);
		string ins;			
		while(getline(inb, ins)) if(ins.size()>0 && ins[0]!='#') {
			
			
			//cout<<ins<<endl;
			deque<double> ds;
			cast_string_to_doubles(ins, ds);

			/*cout<<"ds: ";
			prints(ds);*/
		
			int innum1=cast_int(ds[0]);
			int innum2=cast_int(ds[1]);
					
			double w=1;
			int multiple_l=1;
						
			//--------------------------------------------------------------
			//--------------------------------------------------------------
			if(ds.size()>=4) {
				
				
				
				if(paras.weighted==false)  {
				
					if(ds[2]>0.99) {
						//cout<<ds[2]<<"<- "<<endl;
						multiple_l=cast_int(ds[2]);			
					
					}
				
				}
				
				
				if(paras.weighted==true) {
					
					
					//---------------------------------------------
					if(ds[2]>0) {
						w=ds[2];
					}
					else {
						cerr<<"error: not positive weights"<<endl;
						return false;
					}
					//---------------------------------------------
					
					
					
					if(ds[3]>0.99) {
						//cout<<ds[2]<<"<- "<<endl;
						multiple_l=cast_int(ds[3]);			
					
					}
					//---------------------------------------------
				
				}
			
			}
			
			if(ds.size()==3) {
				
				
				if(paras.weighted) {
				
					if(ds[2]>0)
						w=ds[2];
					else {
						cerr<<"error: not positive weights"<<endl;
						return false;				
					}
				} else {
				
					if(ds[2]>0.99)
						multiple_l=cast_int(ds[2]);
				}
																								
			}
			//--------------------------------------------------------------
			//--------------------------------------------------------------
					
			int new1=newlabels[innum1];
			int new2=newlabels[innum2];
			
			
			if(new1!=new2 && w>0) {		// no self loops!
				
				//cout<<"W: "<<w<<endl;
				
				vertices[new1]->links->push_back(new2, multiple_l, w);
				vertices[new2]->links->push_back(new1, multiple_l, w);
				
			}

		}
		
		
		oneM=0;

		for(int i=0; i<dim; i++) {
		
			vertices[i]->links->freeze();
			
			int stub_number_i=0;
			double strength_i=0;
			for(int j=0; j<vertices[i]->links->size(); j++) {
				
				stub_number_i+=vertices[i]->links->w[j].first;
				
				//cout<<"-> "<<vertices[i]->links->w[j].second<<endl;
				strength_i+=vertices[i]->links->w[j].second;
			
			
			}
			
			
			vertices[i]->stub_number=stub_number_i;
			vertices[i]->strength=strength_i;
			oneM+=stub_number_i;
				
		}
		



	}
	else
		cerr<<"File corrupted"<<endl;
	
	//draw("before_set");
	if(paras.weighted)
		set_proper_weights();

	
	
	return good_file;
	
}


void static_network::set_graph(deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node, deque<int> & label_rows) {
	
	
	
	clear();
	
	
	// there is no check between label_rows and link per node but they need to have the same labels
	// link_per_node and weights_per_node are the list of links and weights. label_rows[i] is the label corresponding to row i
	
		
		
	map<int, int> newlabels;		// this maps the old labels with the new one
	for(int i=0; i<int(label_rows.size()); i++)
		newlabels.insert(make_pair(label_rows[i], newlabels.size()));

		
		
	dim=newlabels.size();
	

	
	for(int i=0; i<dim; i++)
		vertices.push_back(new vertex (0, 0, link_per_node[i].size()));
	
	
	for(map<int, int>::iterator itm=newlabels.begin(); itm!=newlabels.end(); itm++)
		vertices[itm->second]->id_num=itm->first;
			
	
			
	
	for(int i=0; i<int(link_per_node.size()); i++) {
			
		
		for(int j=0; j<int(link_per_node[i].size()); j++) {
			
			int new2=newlabels[link_per_node[i][j]];
			vertices[i]->links->push_back(new2, weights_per_node[i][j].first, weights_per_node[i][j].second);
			
		}
	
	}
	
	oneM=0;

	for(int i=0; i<dim; i++) {
	
		vertices[i]->links->freeze();
		
		
		int stub_number_i=0;
		double strength_i=0;
		for(int j=0; j<vertices[i]->links->size(); j++) {
			
			stub_number_i+=vertices[i]->links->w[j].first;
			strength_i+=vertices[i]->links->w[j].second;
		
		
		}
		
		
		vertices[i]->stub_number=stub_number_i;
		vertices[i]->strength=strength_i;
		oneM+=stub_number_i;
			
		
		
	}
	

	//draw_consecutive();
	//draw("GIIO");
	if(paras.weighted)
		set_proper_weights();




}




int static_network::kin_m (const deque<int> & seq) {
	
	
	if(seq.size()>double(oneM)/dim) {
	
		set<int> H;
		deque_to_set(seq, H);
		return kin_m(H);
	
	}
	
	
	int k=0;
	for (int i=0; i<int(seq.size()); i++)
		k+=vertices[seq[i]]->kplus_m(seq);

	return k;

}




int static_network::ktot_m (const deque<int> &seq) {
	
	int k=0;
	for (int i=0; i<int(seq.size()); i++)
		k+=vertices[seq[i]]->stub_number;
	return k;

}


int static_network::ktot_m(const set <int> &s) {
	
	int k=0;
	for (set<int>::iterator it=s.begin(); it!=s.end(); it++)
		k+=vertices[*it]->stub_number;
	
	
	
	return k;

}


int static_network::kin_m(const set <int> &s) {
	
	int k=0;
	for (set<int>::iterator it=s.begin(); it!=s.end(); it++)
		k+=vertices[*it]->kplus_m(s);
	
	
	
	return k;

}





int static_network::draw(string file_name) {
	
	
	
		int h= file_name.size();
		
		char b[h+1];
		for (int i=0; i<h; i++)
			b[i]=file_name[i];
		b[h]='\0';
		
		
		
		ofstream graph_out(b);

		
		if (paras.weighted) {
			
			for (int i=0; i<int(vertices.size()); i++)
				for (int j=0; j<vertices[i]->links->size(); j++) if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num)
					graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->links->l[j]]->id_num<<"\t"<<vertices[i]->original_weights[j]<<"\t"<<vertices[i]->links->w[j].first<<"\t"<<endl;
		
		}
		else {
			
			for (int i=0; i<int(vertices.size()); i++)
				for (int j=0; j<vertices[i]->links->size(); j++) if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num)
					graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->links->l[j]]->id_num<<"\t"<<vertices[i]->links->w[j].first<<endl;
		
		}
		
		
	return 0;

}




void static_network::get_id_label (map <int, int> &a) {
	
	for (int i=0; i<dim; i++)
		a.insert(make_pair(vertices[i]->id_num, i));


}

void static_network::deque_id(deque<int> & a) {
	
	for(int i=0; i<int(a.size()); i++)	
		a[i]=vertices[a[i]]->id_num;


}

void static_network::print_id(const deque<int> & a, ostream & pout) {
	
	for (int i=0; i<int(a.size()); i++)
		pout<<vertices[a[i]]->id_num<<"\t";
	pout<<endl;


}


void static_network::print_id(const set<int> & a, ostream & pout) {
	
	for (set<int>::iterator its=a.begin(); its!=a.end(); its++)
		pout<<vertices[*its]->id_num<<"\t";
	pout<<endl;


}



void static_network::print_id(const deque<deque<int> > & a, ostream & pout) {
	
	for(int i=0; i<int(a.size()); i++)
		print_id(a[i], pout);
	


}

void static_network::print_id(const deque<set<int> > & a, ostream & pout) {
	
	for(int i=0; i<int(a.size()); i++)
		print_id(a[i], pout);
	


}




int static_network::translate_anyway(deque<deque<int> > & ten) {

	map<int, int> A;
	get_id_label(A);
	
	deque<deque<int> > ten2;
	
	for(int i=0; i<int(ten.size()); i++) {
	
		deque<int> ff;
		
		for(int j=0; j<int(ten[i].size()); j++) {
			
			map<int, int>::iterator itf=A.find(ten[i][j]);
			if(itf!=A.end())
				ff.push_back(itf->second);
			
			
			
		}
		
		if(ff.size()>0)
			ten2.push_back(ff);
		
	
	}
	
	ten=ten2;
	
	return 0;


}



int static_network::translate(deque<deque<int> > & ten) {

	map<int, int> A;
	get_id_label(A);
	
	deque<deque<int> > ten2;
	
	for(int i=0; i<int(ten.size()); i++) {
	
		deque<int> ff;
		
		for(int j=0; j<int(ten[i].size()); j++) {
			
			map<int, int>::iterator itf=A.find(ten[i][j]);
			if(itf==A.end()) {
				
				cerr<<"warning: the nodes in the communities are different from those ones in the network!"<<endl;
				//return -1;
			
			
			}
			else
				ff.push_back(itf->second);
			
			
			
		}
		
		if(ff.size()>0)
			ten2.push_back(ff);
		
	
	}
	
	ten=ten2;
	
	return 0;


}




void static_network::set_subgraph(deque<int> & group, deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node) {
	
	
	// in this function I'm not using id's... because I want to work with the same labels (don't want to translate)
	
	
	
	sort(group.begin(), group.end());

	weights_per_node.clear();
	link_per_node.clear();
	
	for(int i=0; i<int(group.size()); i++) {
		
		int nodei=group[i];
		
		
		deque<int> link_i;
		deque<pair<int, double> > weight_i;
		
		
		for(int j=0; j<vertices[nodei]->links->size(); j++) if(binary_search(group.begin(), group.end(), vertices[nodei]->links->l[j])) {
			
			link_i.push_back(vertices[nodei]->links->l[j]);
			//weight_i.push_back(make_pair(vertices[nodei]->links->w[j].first,   vertices[nodei]->original_weights[j]));
			if(paras.weighted)
				weight_i.push_back(make_pair(vertices[nodei]->links->w[j].first, vertices[nodei]->original_weights[j]));
			else
				weight_i.push_back(make_pair(vertices[nodei]->links->w[j].first, 1));
			//weight_i.push_back(make_pair(vertices[nodei]->links->w[j].first, vertices[nodei]->links->w[j].second));
		}
		
		
		link_per_node.push_back(link_i);
		weights_per_node.push_back(weight_i);
		
		
	}
	
	


}




void static_network::set_proper_weights() {

	
	
	// this function id to normalize the weights in order to have the -log(prob(Weiight>w)) which is simply w[i].second / <w_ij>

	
	
	if(dim==0) {
		//cout<<"network empty"<<endl;
		//cherr();
	} else {
	
				
		
		for(int i=0; i<dim; i++) {
		
			vertices[i]->original_weights.clear();
			
			for(int j=0; j<vertices[i]->links->size(); j++) {
				vertices[i]->original_weights.push_back(vertices[i]->links->w[j].second);
			}
		}
		
		
		for(int i=0; i<dim; i++) {
			
			for(int j=0; j<vertices[i]->links->size(); j++) {
				
				double w1= (vertices[i]->strength / vertices[i]->stub_number) * vertices[i]->links->w[j].first;
				double w2 = (vertices[vertices[i]->links->l[j]]->strength / vertices[vertices[i]->links->l[j]]->stub_number) * vertices[i]->links->w[j].first;
				vertices[i]->links->w[j].second/=  2./(1./w1 + 1./w2);
								 
			}
			
		}		
	
	
	}

}





void static_network::set_connected_components(deque<deque<int> > & comps) {

	
	
	comps.clear();
	set<int> not_assigned;
	for(int i=0; i<dim; i++)
		not_assigned.insert(i);
	
	while(not_assigned.size()>0) {
	
		
		int source = *not_assigned.begin();
		
		set<int> mates;
		same_component(source, mates);
		
		
		deque<int> ccc;
		for(set<int>::iterator its=mates.begin(); its!=mates.end(); its++) {
			ccc.push_back(*its);
			not_assigned.erase(*its);
			
		}
		
		comps.push_back(ccc);
	}
		



}




void static_network::same_component(int source, set<int> & already_gone) {
	
	
	already_gone.clear();
	already_gone.insert(source);
	
	deque<int> this_shell;
	this_shell.push_back(source);
	
	
	while(this_shell.size()>0) {
		
		deque<int> next_shell;

		for(int i=0; i<int(this_shell.size()); i++) for(int j=0; j<vertices[this_shell[i]]->links->size(); j++) {
			
			
			if(already_gone.insert(vertices[this_shell[i]]->links->l[j]).second==true)
				next_shell.push_back(vertices[this_shell[i]]->links->l[j]);
		}
		
		this_shell=next_shell;
		
		
	}
	
}




int static_network::propagate_distances(deque<int> & new_shell, set<int> & already_gone, deque<pair<int, int> > & distances_node, int shell, deque<double> & ML, int & reached, int step) {

	shell++;
	deque<int> next_shell;

	
	
	for(int i=0; i<int(new_shell.size()); i++) for(int j=0; j<vertices[new_shell[i]]->links->size(); j++) {
		
		
		if(already_gone.insert(vertices[new_shell[i]]->links->l[j]).second==true) {
			
			distances_node.push_back(make_pair(shell, vertices[new_shell[i]]->links->l[j]));
			next_shell.push_back(vertices[new_shell[i]]->links->l[j]);
		
		}
		
		
		
	
	}

	
	
	/*
	cout<<"new shell "<<shell<<endl;
	print_id(next_shell, cout);
	prints(ML);
	*/
	
	
	
		
	if(next_shell.size()>0) {
		
		
		
		if(shell>=int(ML.size()))
			ML.push_back(dim*step);
	
		reached+=next_shell.size();
		ML[shell]+=reached;

		
		
		return propagate_distances(next_shell, already_gone, distances_node, shell, ML, reached, step);
	
	
	}
	
	return 0;
	

}








int static_network::draw_consecutive(string file_name1, string file_name2) {	
	
	
	char b[128];
	cast_string_to_char(file_name1, b);
	
	
	//cout<<"drawing in file "<<b<<endl;
	ofstream graph_out(b);
	
	if(paras.weighted)	{
		for (int i=0; i<int(vertices.size()); i++)
			for (int j=0; j<vertices[i]->links->size(); j++) if(i < vertices[i]->links->l[j])
				graph_out<<i<<"\t"<<vertices[i]->links->l[j]<<"\t"<<cast_int(vertices[i]->original_weights[j])<<endl;
	} else {
	
		for (int i=0; i<int(vertices.size()); i++)
			for (int j=0; j<vertices[i]->links->size(); j++) if(i < vertices[i]->links->l[j])
				graph_out<<i<<"\t"<<vertices[i]->links->l[j]<<"\t"<<vertices[i]->links->w[j].first<<endl;
	}
	

	char bb[128];

	cast_string_to_char(file_name2, bb);
	ofstream graph_out2(bb);
	for (int i=0; i<int(vertices.size()); i++)
		graph_out2<<i<<" "<<vertices[i]->id_num<<endl;

		

	return 0;

}






int static_network::set_upper_network(map<int, map<int, pair<int, double> > > & neigh_weight_f, module_collection & Mcoll) {
	
	
	// loop on all the edges of the network...

	neigh_weight_f.clear();

	if(Mcoll.size()==0)
		return 0;
	
	
	
	
	map<int, map<int, pair<double, double> > >neigh_weight_s;


	
	for(map<int, double >::iterator its = Mcoll.module_bs.begin(); its!=Mcoll.module_bs.end(); its++) {

		
		map<int, pair<double, double> > neigh_weight;
		map<int, pair<int, double> > ooo;
		neigh_weight_s.insert(make_pair(its->first, neigh_weight));
		neigh_weight_f.insert(make_pair(its->first, ooo));
		
	}
	
	
	
	for(int i=0; i<dim; i++) {
		
		set<int> & mem1= Mcoll.memberships[i];
		
		for(int j=0; j<vertices[i]->links->size(); j++) {
			
			int & neigh = vertices[i]->links->l[j];
			set<int> & mem2= Mcoll.memberships[neigh];
			
			double denominator = mem1.size() * mem2.size();
			// I add a link between all different modules
			
			if(paras.weighted) {	for(set<int>:: iterator itk=mem1.begin(); itk!=mem1.end(); itk++) for(set<int>:: iterator itkk=mem2.begin(); itkk!=mem2.end(); itkk++) if(*itk!=*itkk)
				int_histogram(*itkk, neigh_weight_s[*itk], double(vertices[i]->links->w[j].first)/denominator, vertices[i]->original_weights[j]/denominator);}
			else {					for(set<int>:: iterator itk=mem1.begin(); itk!=mem1.end(); itk++) for(set<int>:: iterator itkk=mem2.begin(); itkk!=mem2.end(); itkk++) if(*itk!=*itkk)
				int_histogram(*itkk, neigh_weight_s[*itk], double(vertices[i]->links->w[j].first)/denominator, double(vertices[i]->links->w[j].first)/denominator);}

			
		}
	
	}
	
	
	
	for(map<int, map<int, pair<double, double> > >:: iterator itm= neigh_weight_s.begin(); itm!= neigh_weight_s.end(); itm++) {
		
		for(map<int, pair<double, double> >::iterator itm_ = itm->second.begin(); itm_!=itm->second.end(); itm_++) if(itm_->first < itm->first) {
			
			
			int intml=cast_int(itm_->second.first);
			if(intml>0) {
				neigh_weight_f[itm->first].insert(make_pair(itm_->first, make_pair( intml, itm_->second.second)));
				neigh_weight_f[itm_->first].insert(make_pair(itm->first, make_pair( intml, itm_->second.second)));
			}
			
			
		
		}
	
	}
	
	
	
	
	
	
	return 0;
	
}








int static_network::draw_with_weight_probability(string file_name) {
	
	

	int h= file_name.size();
	
	char b[h+1];
	for (int i=0; i<h; i++)
		b[i]=file_name[i];
	b[h]='\0';
	
	
	
	ofstream graph_out(b);

	
	if (paras.weighted) {
		
		for (UI i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->links->size(); j++)
				graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->links->l[j]]->id_num<<"\t"<<vertices[i]->original_weights[j]<<"\t"<<vertices[i]->links->w[j].first<<endl;
	
	} 	
	
	
		
	
	
	return 0;

}



void static_network::print_degree_of_homeless(DI & homel, ostream & outt) {
	
	deque<int> degree_of_homeless;
	for(UI i=0; i<homel.size(); i++)
		degree_of_homeless.push_back(vertices[homel[i]]->stub_number);
	
	outt<<"average degree of homeless nodes: "<<average_func(degree_of_homeless)<<" dev: "<<sqrt(variance_func(degree_of_homeless))<<endl;



}



#endif




