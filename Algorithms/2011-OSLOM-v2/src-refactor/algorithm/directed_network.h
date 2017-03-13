#if !defined(STATIC_static_network_INCLUDED)
#define STATIC_static_network_INCLUDED


#include "./standard_package/standard_include.cpp"
#include "wsarray.h"






class static_network {
	
	
	
	public:
		
		
		static_network(){};
		~static_network();
		

		//int draw(string, bool);
		int draw(string);
		int draw_consecutive(string file_name1, string file_name2);
		int draw_with_weight_probability(string file_name);

		void print_id(const deque<int> & a, ostream &);
		void print_id(const deque<deque<int> > & , ostream &);
		void print_id(const deque<set<int> > & , ostream &);
		void print_id(const set<int> & , ostream & );
		void deque_id(deque<int> & );
		
		void set_subgraph(deque<int> & group, deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node);
		
		
		
		int translate(deque<int> & a);
		int translate(deque<deque<int> > &);
		int translate_anyway(deque<deque<int> > & ten);

		void get_id_label(map <int, int> &);
		int id_of(int a) {return vertices[a]->id_num;};
		
	
		int size() {return dim;};
		double stubs() {return oneM;};
		
		int kin_m (const deque<int> &);
		int kin_m(const set<int> &);
		pair<int, int> ktot_m (const deque<int> &);
		pair<int, int> ktot_m (const set<int> &);
		
	

		void set_graph(map<int, map<int, pair<int, double> > > & A);
		bool set_graph(string file_name);
		void set_graph(deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node, deque<int> & label_rows);
		void clear();
		
		void set_proper_weights();
		
		
		void set_connected_components(deque<deque<int> > & );
		int propagate_distances(deque<int> & new_shell, set<int> & already_gone, deque<pair<int, int > > & distances_node, int shell, deque<double> & ML, int &, int);
		void same_component(int , set<int> &);
		
		
		
		int set_upper_network(map<int, map<int, pair<int, double> > > & neigh_weight_f, module_collection & module_coll);

		void print_degree_of_homeless(DI & homel, ostream & outt);
		
	protected:



		class  vertex {
				
			public:
				
				vertex(int b, int c, int preall_i, int preall_o);
				~vertex();
						
				pair<int, int> kplus_m(const deque<int> &);
				pair<double, double> kplus_w(const deque<int> &);
				
				pair<int, int> kplus_m(const set<int> &);

							
				int id_num;							// id
				
				double instrength;					// sum of the weights
				int instub_number;					// number of stubs
				double outstrength;					// sum of the weights
				int outstub_number;					// number of stubs
				
				wsarray* inlinks;					// array with label of neighbor, multiple links, sum of the weights towards it
				wsarray* outlinks;					// array with label of neighbor, multiple links, sum of the weights towards it
				//deque<double> in_original_weights;
				deque<double> out_original_weights;

		};

				
				
		int dim;									// number of nodes
		int oneM;									// number of in(out)-stubs
		
		deque <vertex*> vertices;
		
		void set_oneM_etc();		

		
		
		
		
};









static_network::vertex::vertex(int b, int c, int preall_o, int preall_i) {
	
	id_num=b;
	outlinks=new wsarray(preall_o);
	inlinks=new wsarray(preall_i);
	
}



static_network::vertex::~vertex() {
	
	delete inlinks;
	inlinks=NULL;
	
	delete outlinks;
	outlinks=NULL;
	
	
	
}



pair<int, int> static_network::vertex::kplus_m(const deque<int> &a) {

	// computes the internal degree of the vertex respect with a
	
	int ins=0;
	//double f=0;
	for (UI i=0; i<a.size(); i++) {
		pair<int, double> A=inlinks->posweightof(a[i]).second;
		ins+=A.first;
	}
	
	int outs=0;
	//double f=0;
	for (UI i=0; i<a.size(); i++) {
		pair<int, double> A=outlinks->posweightof(a[i]).second;
		outs+=A.first;
	}

	
	
	return make_pair(ins, outs);
	
}




pair<double, double> static_network::vertex::kplus_w(const deque<int> &a) {
	
	// computes the internal degree of the vertex respect with a
	
	double ins=0;
	//double f=0;
	for (UI i=0; i<a.size(); i++) {
		pair<int, double> A=inlinks->posweightof(a[i]).second;
		ins+=A.second;
	}
	
	double outs=0;
	//double f=0;
	for (UI i=0; i<a.size(); i++) {
	
		pair<int, double> A=outlinks->posweightof(a[i]).second;
		//cout<<a[i]<<" -*-*-* "<<A.first<<" "<<A.second<<endl;
		
		outs+=A.second;
	}

	
	
	return make_pair(ins, outs);
	
	
	
}




pair<int, int> static_network::vertex::kplus_m(const set<int> &a) {
	
	// computes the internal degree of the vertex respect with a (a is supposed to be sorted)
	
	int ins=0;
	//double f=0;
	
	for (int i=0; i<inlinks->size(); i++) {
		if(a.find(inlinks->l[i])!=a.end()) {
			
			ins+=inlinks->w[i].first;
		}
	}
	
	
	
	int outs=0;
	for (int i=0; i<outlinks->size(); i++) {
		if(a.find(outlinks->l[i])!=a.end()) {
			outs+=outlinks->w[i].first;
		}
	}

	
	
	return make_pair(ins, outs);
	
}


static_network::~static_network() {
	
	clear();

}


void static_network::clear() {

	
	for (UI i=0; i<vertices.size(); i++) {
		
		delete vertices[i];
		vertices[i]=NULL;
	
	}
	
	vertices.clear();
	dim=0;
	oneM=0;
	
	
	

}





void static_network::set_graph(map<int, map<int, pair<int, double> > > & A) {
	
	// this maps the id into the usual stuff neighbors-weights
	// A gives the out-links				 ****IMPORTANT****
	
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
	printm(weights_per_node);//*/
	
	
	set_graph(link_per_node, weights_per_node, label_rows);


}



//   all this stuff here should be improved
//   all this stuff here should be improved
//   all this stuff here should be improved



void static_network::set_oneM_etc() {

	
		
	
	
	oneM=0;

	for(int i=0; i<dim; i++) {
	
		vertices[i]->inlinks->freeze();
		vertices[i]->outlinks->freeze();
		
		int stub_number_i=0;
		double strength_i=0;
		
		for(int j=0; j<vertices[i]->inlinks->size(); j++) {
			stub_number_i+=vertices[i]->inlinks->w[j].first;
			strength_i+=vertices[i]->inlinks->w[j].second;
			//cout<<"-> "<<vertices[i]->links->w[j].second<<endl;			
		}
		vertices[i]-> instub_number=stub_number_i;
		vertices[i]-> instrength=strength_i;
		
		
		
		stub_number_i=0;
		strength_i=0;
		
		for(int j=0; j<vertices[i]->outlinks->size(); j++) {
			stub_number_i+=vertices[i]->outlinks->w[j].first;
			strength_i+=vertices[i]->outlinks->w[j].second;
		}
		vertices[i]-> outstub_number=stub_number_i;
		vertices[i]-> outstrength=strength_i;
					
		oneM+=stub_number_i;
			
	
	}
	
	


}



bool static_network::set_graph(string file_name) {
	
	
	clear();
	
	
	char b[file_name.size()+1];
	cast_string_to_char(file_name, b);
	
	
	
	
	map<int, int> newlabels;
	deque<deque<int> >  link_per_node;
	deque<deque<pair<int, double> > >  weights_per_node;
	deque<int> label_rows;
	
	
	
	
	int label=0;
		
	
	ifstream inb(b);
	string ins;
		
		
	while(getline(inb, ins)) if(ins.size()>0 && ins[0]!='#') {
			
			
		deque<double> ds;
		cast_string_to_doubles(ins, ds);

		if(ds.size()<2) {
			cerr<<"From file "<<file_name<<": string not readable "<<ins<<" "<<endl;
			return false;
		}
				
				
		//prints(ds);
		//cout<<"-------------------------------------"<<endl;
		
		
		int innum1=cast_int(ds[0]);
		int innum2=cast_int(ds[1]);
		
		if(innum1!=innum2) {
			
			if (newlabels.find(innum1)==newlabels.end()) {
			
				newlabels.insert(make_pair(innum1, label++));
				label_rows.push_back(innum1);
				deque<int> first;
				link_per_node.push_back(first);
				deque<pair<int, double> > sec;
				weights_per_node.push_back(sec);		
			}
			
			
						
			if (newlabels.find(innum2)==newlabels.end()) {
			
				newlabels.insert(make_pair(innum2, label++));
				label_rows.push_back(innum2);
				
				deque<int> first;
				link_per_node.push_back(first);
				deque<pair<int, double> > sec;
				weights_per_node.push_back(sec);		
			}

			
			
			
			int pos=newlabels[innum1];
			link_per_node[pos].push_back(innum2);
				
							
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
			
			weights_per_node[pos].push_back(make_pair(multiple_l, w));
			
		}
	
	}
	
	
	/*
	prints(label_rows);
	printm(link_per_node);
	//printm(weights_per_node);
	//*/
	

	
	set_graph(link_per_node, weights_per_node, label_rows);
	
	return true;
	
}


void static_network::set_graph(deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node, deque<int> & label_rows) {
	
	
	
	clear();
	
	
	
	// there is no check between label_rows and link per node but they need to have the same labels
	// link_per_node and weights_per_node are the list of out-links and weights. label_rows[i] is the label corresponding to row i
	
		
		
	map<int, int> newlabels;		// this maps the old labels with the new one
	for(UI i=0; i<label_rows.size(); i++)
		newlabels.insert(make_pair(label_rows[i], newlabels.size()));

		
		
	dim=newlabels.size();
	
	
	deque<int> inlinks_pernode;		// this counts how many in-links a given nodes receives
	for(int i=0; i<dim; i++)
		inlinks_pernode.push_back(0);
	

	for(UI i=0; i<link_per_node.size(); i++)
		for(UI j=0; j<link_per_node[i].size(); j++)
			inlinks_pernode[newlabels[link_per_node[i][j]]]++;
	
	
	for(int i=0; i<dim; i++)
		vertices.push_back(new vertex (0, 0, link_per_node[i].size(), inlinks_pernode[i]));
	
	
	for(map<int, int>::iterator itm=newlabels.begin(); itm!=newlabels.end(); itm++)
		vertices[itm->second]->id_num=itm->first;
			
	
			
	
	for(UI i=0; i<link_per_node.size(); i++) {
			
		
		for(UI j=0; j<link_per_node[i].size(); j++) {
			
			int new2=newlabels[link_per_node[i][j]];
			vertices[i]->outlinks->push_back(new2, weights_per_node[i][j].first, weights_per_node[i][j].second);
			vertices[new2]->inlinks->push_back(i, weights_per_node[i][j].first, weights_per_node[i][j].second);
			
		}
	
	}
	
	
	set_oneM_etc();
	
	
	
	
		
	

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
	
	
	int ki=0;
	//int ko=0;
	for (UI i=0; i<seq.size(); i++) {
		ki+=vertices[seq[i]]->kplus_m(seq).first;
		//ko+=vertices[seq[i]]->kplus_m(seq).second;
	
	}
	
	
	return ki;

}



pair<int, int> static_network::ktot_m (const deque<int> &seq) {
	
	int ki=0;
	int ko=0;
	
	for (UI i=0; i<seq.size(); i++) {
		
		ki+=vertices[seq[i]]->instub_number;
		ko+=vertices[seq[i]]->outstub_number;
	
	}
	return make_pair(ki, ko);

}




pair<int, int> static_network::ktot_m(const set <int> &s) {
	
	int ki=0;
	int ko=0;	
	
	for (set<int>::iterator it=s.begin(); it!=s.end(); it++) {
		
		ki+=vertices[*it]->instub_number;
		ko+=vertices[*it]->outstub_number;
	
	
	}
	
	
	return make_pair(ki, ko);

}


int static_network::kin_m(const set <int> &s) {
	
	int ki=0;
	
	for (set<int>::iterator it=s.begin(); it!=s.end(); it++)
		ki+=vertices[*it]->kplus_m(s).first;
	
	
	
	return ki;

}





int static_network::draw_consecutive(string file_name1, string file_name2) {
	
	
	char b[128];
	cast_string_to_char(file_name1, b);
	
	
	//cout<<"drawing in file "<<b<<endl;
	ofstream graph_out(b);
	
		
				
		
	
	
	if(paras.weighted)	{
		for (UI i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->outlinks->size(); j++)
				graph_out<<i<<"\t"<<vertices[i]->outlinks->l[j]<<"\t"<<cast_int(vertices[i]->out_original_weights[j])<<endl;
	} else {
				
		for (UI i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->outlinks->size(); j++)
				graph_out<<i<<"\t"<<vertices[i]->outlinks->l[j]<<"\t"<<vertices[i]->outlinks->w[j].first<<endl;
	}
	

		
	
		
	char bb[128];

	cast_string_to_char(file_name2, bb);
	ofstream graph_out2(bb);
	for (UI i=0; i<vertices.size(); i++)
		graph_out2<<i<<" "<<vertices[i]->id_num<<endl;

		

	return 0;

}






int static_network::draw(string file_name) {
	
	

	int h= file_name.size();
	
	char b[h+1];
	for (int i=0; i<h; i++)
		b[i]=file_name[i];
	b[h]='\0';
	
	
	
	ofstream graph_out(b);

	
	if (paras.weighted) {
		
		for (UI i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->outlinks->size(); j++)
				graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->outlinks->l[j]]->id_num<<"\t"<<vertices[i]->out_original_weights[j]<<"\t"<<vertices[i]->outlinks->w[j].first<<endl;
	
	} else {
		
		for (UI i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->outlinks->size(); j++)
				graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->outlinks->l[j]]->id_num<<"\t"<<vertices[i]->outlinks->w[j].first<<endl;
				//"\t"<<vertices[i]->out_original_weights[j]<<"\t"<<vertices[i]->outlinks->w[j].second<<endl;
	
	}
	
	
	
	/*
	for (int i=0; i<vertices.size(); i++)
		for (int j=0; j<vertices[i]->inlinks->size(); j++)
			cout<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->inlinks->l[j]]->id_num<<"\t"<<vertices[i]->inlinks->w[j].first<<
			"\t"<<77<<"\t"<<vertices[i]->inlinks->w[j].second<<endl;

	*/
	
	
	
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
			for (int j=0; j<vertices[i]->outlinks->size(); j++)
				graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->outlinks->l[j]]->id_num<<"\t"<<vertices[i]->out_original_weights[j]<<"\t"<<vertices[i]->outlinks->w[j].first<<" "<<vertices[i]->outlinks->w[j].second<<endl;
	
	} else {
		
		for (UI i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->outlinks->size(); j++)
				graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->outlinks->l[j]]->id_num<<"\t"<<vertices[i]->outlinks->w[j].first<<endl;
				//"\t"<<vertices[i]->out_original_weights[j]<<"\t"<<vertices[i]->outlinks->w[j].second<<endl;
	
	}
	
	
	
	/*
	for (int i=0; i<vertices.size(); i++)
		for (int j=0; j<vertices[i]->inlinks->size(); j++)
			cout<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->inlinks->l[j]]->id_num<<"\t"<<vertices[i]->inlinks->w[j].first<<
			"\t"<<77<<"\t"<<vertices[i]->inlinks->w[j].second<<endl;

	*/
	
	
	
	return 0;

}



void static_network::get_id_label (map <int, int> &a) {
	
	for (int i=0; i<dim; i++)
		a.insert(make_pair(vertices[i]->id_num, i));


}

void static_network::deque_id(deque<int> & a) {
	
	for(UI i=0; i<a.size(); i++)	
		a[i]=vertices[a[i]]->id_num;


}

void static_network::print_id(const deque<int> & a, ostream & pout) {
	
	for (UI i=0; i<a.size(); i++)
		pout<<vertices[a[i]]->id_num<<"\t";
	pout<<endl;


}


void static_network::print_id(const set<int> & a, ostream & pout) {
	
	for (set<int>::iterator its=a.begin(); its!=a.end(); its++)
		pout<<vertices[*its]->id_num<<"\t";
	pout<<endl;


}



void static_network::print_id(const deque<deque<int> > & a, ostream & pout) {
	
	for(UI i=0; i<a.size(); i++)
		print_id(a[i], pout);
	


}

void static_network::print_id(const deque<set<int> > & a, ostream & pout) {
	
	for(UI i=0; i<a.size(); i++)
		print_id(a[i], pout);
	


}



int static_network::translate(deque<deque<int> > & ten) {

	map<int, int> A;
	get_id_label(A);
	
	deque<deque<int> > ten2;
	
	for(UI i=0; i<ten.size(); i++) {
	
		deque<int> ff;
		
		for(UI j=0; j<ten[i].size(); j++) {
			
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





int static_network::translate(deque<int> & ten) {

	map<int, int> A;
	get_id_label(A);
	
	
	for(UI i=0; i<ten.size(); i++) {
	
		map<int, int>::iterator itf=A.find(ten[i]);
		if(itf==A.end()) {
				
			cerr<<"warning: the nodes in the communities are different from those ones in the network!"<<endl;
			//return -1;
			
			
		}
		else
			ten[i]=itf->second;
			
			
			
	}
		
		
	
	
	
	return 0;


}



void static_network::set_subgraph(deque<int> & group, deque<deque<int> > & link_per_node, deque<deque<pair<int, double> > > & weights_per_node) {
	
	
	// in this function I'm not using id's... because I want to work with the same labels (don't want to translate)
	
	
	
	sort(group.begin(), group.end());

	weights_per_node.clear();
	link_per_node.clear();
	
	for(UI i=0; i<group.size(); i++) {
		
		int nodei=group[i];
		
		
		deque<int> link_i;
		deque<pair<int, double> > weight_i;
		
		
		for(int j=0; j<vertices[nodei]->outlinks->size(); j++) if(binary_search(group.begin(), group.end(), vertices[nodei]->outlinks->l[j])) {
			
			link_i.push_back(vertices[nodei]->outlinks->l[j]);
			if(paras.weighted)
				weight_i.push_back(make_pair(vertices[nodei]->outlinks->w[j].first, vertices[nodei]->out_original_weights[j]));
			else
				weight_i.push_back(make_pair(vertices[nodei]->outlinks->w[j].first, 1));
				
			//weight_i.push_back(make_pair(vertices[nodei]->links->w[j].first, vertices[nodei]->links->w[j].second));
		}
		
		
		link_per_node.push_back(link_i);
		weights_per_node.push_back(weight_i);
		
		
	}
	
	


}




void static_network::set_proper_weights() {

	
	
	// this function id to normalize the weights in order to have the -log(prob(Weiight>w)) which is simply w[i].second / <w_ij>
	// <w_ij> is s_i * s_j / k_i / k_j / eta
	// eta is <s_i/k_i>

	
	
	if(dim==0) {
		//cout<<"network empty"<<endl;
		//cherr();
	} else {
	
	
		for(int i=0; i<dim; i++) {
		
		
			vertices[i]->out_original_weights.clear();
			
			for(int j=0; j<vertices[i]->outlinks->size(); j++) {
				vertices[i]->out_original_weights.push_back(vertices[i]->outlinks->w[j].second);
			}
					
		
		}
		
		
		
			
		for(int i=0; i<dim; i++) {
			
			for(int j=0; j<vertices[i]->outlinks->size(); j++) {
				
				
				int & neigh= vertices[i]->outlinks->l[j];
				
				double w1= (vertices[i]->outstrength / vertices[i]->outstub_number) * vertices[i]->outlinks->w[j].first;
				double w2 = (vertices[neigh]->instrength / vertices[neigh]->instub_number) * vertices[i]->outlinks->w[j].first;
				
				vertices[i]->outlinks->w[j].second/=  2./(1./w1 + 1./w2) ;
				int posneigh = vertices[neigh]->inlinks->find(i);
				vertices[neigh]->inlinks->w[posneigh].second/=  2./(1./w1 + 1./w2);
				
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

		for(int i=0; i<int(this_shell.size()); i++) {
			
			for(int j=0; j<vertices[this_shell[i]]->inlinks->size(); j++) {
				
				
				if(already_gone.insert(vertices[this_shell[i]]->inlinks->l[j]).second==true)
					next_shell.push_back(vertices[this_shell[i]]->inlinks->l[j]);
				
			}
			
			for(int j=0; j<vertices[this_shell[i]]->outlinks->size(); j++) {
				
				
				if(already_gone.insert(vertices[this_shell[i]]->outlinks->l[j]).second==true)
					next_shell.push_back(vertices[this_shell[i]]->outlinks->l[j]);
			}
		}
		
		this_shell=next_shell;
		
		
	}
	
}


int static_network::propagate_distances(deque<int> & new_shell, set<int> & already_gone, deque<pair<int, int> > & distances_node, int shell, deque<double> & ML, int & reached, int step) {

	shell++;
	deque<int> next_shell;

	
	
	for(UI i=0; i<new_shell.size(); i++) {
		
		for(int j=0; j<vertices[new_shell[i]]->outlinks->size(); j++) {
			if(already_gone.insert(vertices[new_shell[i]]->outlinks->l[j]).second==true) {
				
				distances_node.push_back(make_pair(shell, vertices[new_shell[i]]->outlinks->l[j]));
				next_shell.push_back(vertices[new_shell[i]]->outlinks->l[j]);
		
			}
		}
		
		for(int j=0; j<vertices[new_shell[i]]->inlinks->size(); j++) {
			if(already_gone.insert(vertices[new_shell[i]]->inlinks->l[j]).second==true) {
				
				distances_node.push_back(make_pair(shell, vertices[new_shell[i]]->inlinks->l[j]));
				next_shell.push_back(vertices[new_shell[i]]->inlinks->l[j]);
		
			}
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








int static_network::translate_anyway(deque<deque<int> > & ten) {

	map<int, int> A;
	get_id_label(A);
	
	deque<deque<int> > ten2;
	
	for(UI i=0; i<ten.size(); i++) {
	
		deque<int> ff;
		
		for(UI j=0; j<ten[i].size(); j++) {
			
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









int static_network::set_upper_network(map<int, map<int, pair<int, double> > > & neigh_weight_f, module_collection & module_coll) {
	
	
	
	// loop on all the edges of the network...

	neigh_weight_f.clear();

	if(module_coll.size()==0)
		return 0;
	
	
	
	map<int, map<int, pair<double, double> > >neigh_weight_s;


	
	for(map<int, double>::iterator its = module_coll.module_bs.begin(); its!=module_coll.module_bs.end(); its++) {

		
		//cout<<"NAMES:: "<<m_name<<endl;
		map<int, pair<double, double> > neigh_weight;
		map<int, pair<int, double> > ooo;
		neigh_weight_s.insert(make_pair(its->first, neigh_weight));
		neigh_weight_f.insert(make_pair(its->first, ooo));
		
	}
	
	
	
	for(int i=0; i<dim; i++) {
		
		set<int> & mem1= module_coll.memberships[i];
		
		for(int j=0; j<vertices[i]->outlinks->size(); j++) {
			
			int & neigh = vertices[i]->outlinks->l[j];
			set<int> & mem2= module_coll.memberships[neigh];
			
			double denominator = mem1.size() * mem2.size();
			// I add a link between all different modules
			
			
			//cout<<"denomi "<<denominator<<endl;
			
			
			//**************************************************************************************************
			if(paras.weighted) {
				for(set<int>:: iterator itk=mem1.begin(); itk!=mem1.end(); itk++) for(set<int>:: iterator itkk=mem2.begin(); itkk!=mem2.end(); itkk++) if(*itk!=*itkk) {
				
					int_histogram(*itkk, neigh_weight_s[*itk], double(vertices[i]->outlinks->w[j].first)/denominator, vertices[i]->out_original_weights[j]/denominator);
				}
			} else { 
				
				
				
				for(set<int>:: iterator itk=mem1.begin(); itk!=mem1.end(); itk++) for(set<int>:: iterator itkk=mem2.begin(); itkk!=mem2.end(); itkk++) if(*itk!=*itkk) {
				
					int_histogram(*itkk, neigh_weight_s[*itk], double(vertices[i]->outlinks->w[j].first)/denominator, double(vertices[i]->outlinks->w[j].first)/denominator);
			
				}

			}
			//**************************************************************************************************

		
		}
		
		
	
	}
	
	
	
	for(map<int, map<int, pair<double, double> > >:: iterator itm= neigh_weight_s.begin(); itm!= neigh_weight_s.end(); itm++) {
		
		
		
		for(map<int, pair<double, double> >::iterator itm_ = itm->second.begin(); itm_!=itm->second.end(); itm_++) {
			
			int intml=cast_int(itm_->second.first);
			if(intml>0)
				neigh_weight_f[itm->first].insert(make_pair(itm_->first, make_pair( intml, itm_->second.second)));
		}
	
	}
	
	
	
		
	
	
	
	return 0;
	
}








void static_network::print_degree_of_homeless(DI & homel, ostream & outt) {
	
	deque<int> degree_of_homeless;
	for(UI i=0; i<homel.size(); i++)
		degree_of_homeless.push_back(vertices[homel[i]]->instub_number);
	outt<<"average in-degree of homeless nodes: "<<average_func(degree_of_homeless)<<" dev: "<<sqrt(variance_func(degree_of_homeless))<<endl;
	
	degree_of_homeless.clear();
	for(UI i=0; i<homel.size(); i++)
		degree_of_homeless.push_back(vertices[homel[i]]->outstub_number);
	outt<<"average out-degree of homeless nodes: "<<average_func(degree_of_homeless)<<" dev: "<<sqrt(variance_func(degree_of_homeless))<<endl;
	
	degree_of_homeless.clear();
	for(UI i=0; i<homel.size(); i++)
		degree_of_homeless.push_back(vertices[homel[i]]->instub_number + vertices[homel[i]]->outstub_number);
	outt<<"average in+out-degree of homeless nodes: "<<average_func(degree_of_homeless)<<" dev: "<<sqrt(variance_func(degree_of_homeless))<<endl;
	

}




#endif




