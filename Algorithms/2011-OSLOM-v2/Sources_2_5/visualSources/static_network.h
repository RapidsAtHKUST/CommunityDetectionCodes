#if !defined(STATIC_static_network_INCLUDED)
#define STATIC_static_network_INCLUDED


#include "./standard_package/standard_include.cpp"
#include "wsarray.h"






void statement() {
	
	cout<<"\nTo run the program type \n<program name> -f <filename> [-c(-l) <filename2>]\n"<<endl;
	


}

int parse_command_line(bool & value, string & s, string & s2, int argc, char * argv[]) {




	value=false;	
	
	
	int _arg_ = 1;	
	if (argc <= 1) {
		
		statement();
		return -1;
	}

	
		
	s = argv[_arg_];
	
		
	
		
	if (s== "-f") {
		_arg_++;
		s = argv[_arg_];

	}
	
		
	else {
		
		cout<<"ERROR"<<endl<<endl;
		statement();
		return -1;
	}
	
	_arg_++;
	

	if (argv[_arg_]!='\0')
		s2 = argv[_arg_];
	else
		return 0;
	
	
	
	if (s2== "-c" || s2=="-l") {
		_arg_++;
		s2 = argv[_arg_];
		value=true;
	}
	else {
		
		cout<<"ERROR"<<endl<<endl;
		statement();
		return -1;
	}
	

	return 0;


}

class static_network {
	
	
	
	public:
		
		
		static_network(){};
		static_network(deque<deque<int> > & , deque<int> & );
		static_network(bool , deque<deque<int> > & , deque<deque<double> > & , deque<int> & );
		static_network(string);
		static_network(string file_name, bool & good_file);
		static_network(map<int, map<int, double> > & A);
		~static_network();
		void add_isolated(int id_iso);

		int draw(string, bool);
		int draw(string);
		int draw_consecutive(string , string , bool);
		int draw_consecutive(string , string);

		void print_id(const deque<int> & a, ostream &);
		void print_id(const deque<deque<int> > & , ostream &);
		void print_id(const deque<set<int> > & , ostream &);
		void print_id(const set<int> & , ostream & );
		void deque_id(deque<int> & );
		
		int connected(string );		
		void pajek_print_cluster(string file_name, deque<int> group, int number_of_shells);
		void print_subgraph(string , const deque<int> & );
		void print_component(string file_name, const deque<int> & group);
		void set_subgraph(deque<int> & , deque<deque<int> > & , deque<deque<double> > & );
		
		void print_random_version(string );
		void print_connected_components(ostream &);
		void same_component(int , set<int> &);
		void set_connected_components(deque<deque<int> > & );
		void set_connected_components(deque<set<int> > & );
		
		int translate(deque<deque<int> > &);
		void get_id_label(map <int, int> &);
		int get_degree(deque<int> &);

		int size() {return dim;};
		double edges() {return tstrength;};
		
		double kin (const deque<int> &);
		double kin(const set<int> &);
		double ktot (const deque<int> &);
		double ktot (const set<int> &);
		
		void erase_link(int , int);
		
		double newman_modularity(const double kin_g, const double ktot_g);		
		double newman_modularity(set<int> & s);
		double newman_modularity(deque<set<int> > & );
		double newman_modularity(deque<int> & s);
		double newman_modularity(deque<deque<int> > & );
		
		int compute_betweeness_single_source(int,  map<pair<int, int>, double> &);
		int component_betweeness(int , map<pair<int, int>, double> & , set<int> & );
		int all_betweeness(map<pair<int, int>, double> & );
		
		int id_of(int a) {return vertices[a]->id_num;};
		int set_mem_adj (deque<deque<int> > & );		// unweighted networks!

		int distances_from_i(int node, double &, deque<double> & ML, int);
		int propagate_distances(deque<int> & new_shell, set<int> & already_gone, deque<pair<int, int > > & distances_node, int shell, deque<double> & ML, int &, int);
		int diameter_and_asp(double & average, int & diameter, deque<double> &);
		
		int knn(map <int, double> & );
		int knn(map <int, deque<double > > & knn_hist);

		void clustering_coefficient(map <int, deque<double> > & c_k);
		double clustering_coefficient();



		void set_graph(map<int, map<int, double> > & A);
		void set_graph(multimap<int, int > & A);
		bool set_graph(string file_name);
		void set_graph(bool weighted_, deque<deque<int> > & link_per_node, deque<deque<double> > & weights_per_node, deque<int> & label_rows);
		void set_graph(deque<deque<int> > & link_per_node, deque<int> & label_rows);
		void clear();
		bool GN_bench(int nodes, int modules, double kout, double m_degree, deque<deque<int> > & ten);
		bool set_binary_random(const deque<int> & degrees);
		bool set_random_powlaw(int , double , double , int );
		bool set_random_powlaw_multiple(int num_nodes, double tau, double average_k, int max_degree, bool);	
		bool set_multiple_random(const deque<int> & degrees, bool );
	
		void community_netknn(deque<deque<int> > & );
		void community_net(deque<deque<int> > & ten);
		
		double monte_carlo_asp();

		double draw_gnuplot(map<int, double> & lab_x, map<int, double> & lab_y, ostream & goo, bool internal, deque<deque<int> >  M, double width);
		void draw_pajek(map<int, double> & lab_x, map<int, double> & lab_y, string file_name, deque<deque<int> >  M, double scale_factor);
	
		void draw_pajek_directed(map<int, double> & lab_x, map<int, double> & lab_y, string file_name, deque<deque<int> >  M, double scale_factor, 
												deque<int> & node1, deque<int> & a2, deque<double> & w12);

	protected:



		class  vertex {
				
			public:
				
				vertex(int , int , int);
				~vertex();
						
				double kplus(const deque<int> &);
				double kplus(const set<int> &);
							
				int id_num;
				double strength;
				wsarray* links;
				

		};

				
				
		int dim;									// number of nodes
		double tstrength;							// number of links (weighted)
		bool weighted;
		
		deque <vertex*> vertices;
		
		
		
	private:
	
		void propagate_bw(int* , int* , int, set<int> & , set<int> &, deque<int> & );
		void rewiring(deque<set<int> >& en);
		void next_shell(const set<int> & shell0, set<int> & shell1, set<int> & already);

		
		
};







static_network::vertex::vertex(int b, int c, int preall) {
	
	id_num=b;
	links=new wsarray(preall);
	
	
}



static_network::vertex::~vertex() {
	
	delete links;
	links=NULL;
	
	
	
}



double static_network::vertex::kplus(const deque<int> &a) {

	// computes the internal degree of the vertex respect with a
	
	double f=0;
	for (int i=0; i<a.size(); i++)
		f+=links->posweightof(a[i]).second;
		
		
	return f;
	
}





double static_network::vertex::kplus(const set<int> &a) {
	
	// computes the internal degree of the vertex respect with a (a is supposed to be sorted)
	
	double f=0;
	
	for (int i=0; i<links->size(); i++)
		if(a.find(links->l[i])!=a.end())
			f+=links->w[i];
		
	return f;
	
}






static_network::static_network(string file_name, bool & good_file) {
	
	good_file=set_graph(file_name);

}

static_network::static_network(string file_name) {
	
	set_graph(file_name);

}






static_network::static_network(deque<deque<int> > & link_per_node, deque<int> & label_rows) {
	
	set_graph(link_per_node, label_rows);
	


}



static_network::static_network(bool weighted_, deque<deque<int> > & link_per_node, deque<deque<double> > & weights_per_node, deque<int> & label_rows) {
	
	set_graph(weighted_, link_per_node, weights_per_node, label_rows);

}




static_network::static_network(map<int, map<int, double> > & A) {

	set_graph(A);

}




static_network::~static_network() {
	
	clear();

}


void static_network::clear() {

	
	for (int i=0; i<vertices.size(); i++) {
		
		delete vertices[i];
		vertices[i]=NULL;
	
	}
	
	vertices.clear();
	dim=0;
	tstrength=0;
	
	
	

}


void static_network::set_graph(multimap<int, int> & A) {

	
	map<int, map<int, double> > B;
	
	for(multimap<int, int>:: iterator itm=A.begin(); itm!=A.end(); itm++) {
		
		int m_name=itm->first;
		map<int, double> neigh_weight;
		B.insert(make_pair(m_name, neigh_weight));
		
	}
	
	
	for(multimap<int, int>:: iterator itm=A.begin(); itm!=A.end(); itm++) {
	
		//cout<<"link: "<<itm->first<<" "<<itm->second<<endl;
		
		B[itm->first].insert(make_pair(itm->second, 1.));
		B[itm->second].insert(make_pair(itm->first, 1.));
	
	
	}
	
	
	/*for(map<int, map<int, double> >:: iterator itm= B.begin(); itm!=B.end(); itm++)
		prints(itm->second);
	


	cout<<"ok"<<endl;*/
	set_graph(B);



}



void static_network::set_graph(map<int, map<int, double> > & A) {
	
	// this maps the id into the usual stuff neighbors- weights
	deque<deque<int> > link_per_node;
	deque<deque<double> > weights_per_node;
	deque<int> label_rows;
	
	for(map<int, map<int, double> >:: iterator itm= A.begin(); itm!=A.end(); itm++) {
		
		
		label_rows.push_back(itm->first);
		deque<int> n;
		deque<double> w;
		
		
		for(map<int, double>:: iterator itm2=itm->second.begin(); itm2!=itm->second.end(); itm2++) if(itm2->second>0) {
			
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
	
	
	set_graph(true, link_per_node, weights_per_node, label_rows);


}





bool static_network::set_graph(string file_name) {
	
	
	clear();
	
	
	char b[file_name.size()+1];
	cast_string_to_char(file_name, b);
	
	weighted=false;
	
	
	
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
			
			deque<double> ds;
			cast_string_to_doubles(ins, ds);

	
		
			int innum1=cast_int(ds[0]);
			int innum2=cast_int(ds[1]);
					
			double w=1;
			if(ds.size()>2) {
				if(ds[2]<=0)
					w=1;
				else {
					weighted=true;
					w=ds[2];
				}
			}
															
					
			int new1=newlabels[innum1];
			int new2=newlabels[innum2];
			
			
			if(new1!=new2) {		// no self loops!
				
				vertices[new1]->links->push_back(new2, w);
				vertices[new2]->links->push_back(new1, w);
				
			}

		}
		
		
		tstrength=0;

		for(int i=0; i<dim; i++) {
		
			vertices[i]->links->freeze();
			
			double strength_i=0;
			for(int j=0; j<vertices[i]->links->size(); j++)
				strength_i+=vertices[i]->links->w[j];
			
			vertices[i]->strength=strength_i;
			tstrength+=strength_i;
				
			
			
		}
		


		tstrength=tstrength/2.;

	}
	else
		cerr<<"File corrupted"<<endl;
	
	
	
	
	return good_file;
	
}


void static_network::set_graph(bool weighted_, deque<deque<int> > & link_per_node, deque<deque<double> > & weights_per_node, deque<int> & label_rows) {
	
	
	
	clear();
	
	
	// there is no check between label_rows and link per node but they need to have the same labels
	// link_per_node and weights_per_node are the list of links and weights. label_rows[i] is the label corresponding to row i
	
	weighted=weighted_;
		
		
	map<int, int> newlabels;		// this maps the old labels with the new one
	for(int i=0; i<label_rows.size(); i++)
		newlabels.insert(make_pair(label_rows[i], newlabels.size()));

		
		
	dim=newlabels.size();
	

	
	for(int i=0; i<dim; i++)
		vertices.push_back(new vertex (0, 0, link_per_node[i].size()));
	
	
	for(map<int, int>::iterator itm=newlabels.begin(); itm!=newlabels.end(); itm++)
		vertices[itm->second]->id_num=itm->first;
			
	
			
	
	for(int i=0; i<link_per_node.size(); i++) {
			
		
		for(int j=0; j<link_per_node[i].size(); j++) {
			
			int new2=newlabels[link_per_node[i][j]];
			vertices[i]->links->push_back(new2, weights_per_node[i][j]);
			
		}
	
	}
	
	tstrength=0;

	for(int i=0; i<dim; i++) {
	
		vertices[i]->links->freeze();
		
		double strength_i=0;
		for(int j=0; j<vertices[i]->links->size(); j++)
			strength_i+=vertices[i]->links->w[j];
		
		vertices[i]->strength=strength_i;
		tstrength+=strength_i;
			
		
		
	}
	





	tstrength=tstrength/2.;

}

void static_network::set_graph(deque<deque<int> > & link_per_node, deque<int> & label_rows) {
	
	
	
	// label rows contains the label of the subgraph. The very same labels must be in link_per_node
	
	
	// there is no check between label_rows and link per node but they need to have the same labels
	// link_per_node and weights_per_node are the list of links and weights. label_rows[i] is the label corresponding to row i
	
	
	clear();
	
	
	weighted=false;
		
		
	map<int, int> newlabels;		// this maps the old labels into the new ones
	for(int i=0; i<label_rows.size(); i++)
		newlabels.insert(make_pair(label_rows[i], newlabels.size()));


		
		
	dim=newlabels.size();
	
	
	
	for(int i=0; i<dim; i++)
		vertices.push_back(new vertex (0, 0, link_per_node[i].size()));
	
	
	for(map<int, int>::iterator itm=newlabels.begin(); itm!=newlabels.end(); itm++)
		vertices[itm->second]->id_num=itm->first;
	
	
		
	
			
	
	for(int i=0; i<link_per_node.size(); i++) {
			
		
		for(int j=0; j<link_per_node[i].size(); j++) {
			
			int new2=newlabels[link_per_node[i][j]];
			vertices[i]->links->push_back(new2, 1.);
			
		}
	
	}
	
	tstrength=0;

	for(int i=0; i<dim; i++) {
	
		vertices[i]->links->freeze();
		
		double strength_i=0;
		for(int j=0; j<vertices[i]->links->size(); j++)
			strength_i+=vertices[i]->links->w[j];
		
		vertices[i]->strength=strength_i;
		tstrength+=strength_i;
			
		
		
	}
	


	tstrength=tstrength/2.;

}



int static_network::get_degree(deque<int> &d) {

	d.clear();
	for (int i=0; i<dim; i++)
		d.push_back(vertices[i]->links->size());



	return 0;
}


double static_network::newman_modularity(const double kin_g, const double ktot_g) {
	
	return ((kin_g)/(2.*tstrength)-pow((ktot_g)/(2.*tstrength),2));

}
		
double static_network::newman_modularity(set<int> & s) {

	return newman_modularity(kin(s), ktot(s));

}



double static_network::newman_modularity(deque<set<int> > & Comps) {

	double mm=0;
	for(int i=0; i<Comps.size(); i++)
		mm+=newman_modularity(Comps[i]);
	
	return mm;

}


double static_network::newman_modularity(deque<int> & s) {

	return newman_modularity(kin(s), ktot(s));

}



double static_network::newman_modularity(deque<deque<int> > & Comps) {

	double mm=0;
	for(int i=0; i<Comps.size(); i++)
		mm+=newman_modularity(Comps[i]);
	
	return mm;

}




void static_network::erase_link(int a, int b) {


	double sa =vertices[a]->links->posweightof(b).second;	
	
	vertices[a]->links->erase(b);
	vertices[b]->links->erase(a);

	vertices[a]->strength-=sa;
	vertices[b]->strength-=sa;

	
	tstrength-=sa;



}





int static_network::connected (string str) {

	int spannet=0;
	deque <set<int> > partic;
	deque <int> present;
	present.assign(dim,0);

	
	while (spannet!=dim) {
		
		set <int> connected;
		set <int> newcon;
				

		for (int i=0; i<dim; i++)
			if (present[i]==0) {
				connected.insert(i);
				newcon.insert(i);
				present[i]=1;
				break;
			}
			
		
	
		while (newcon.size()!=0) {
			
			set <int> nnewcon=newcon;
			newcon.clear();
			set <int>::iterator it=nnewcon.begin();
			while (it!=nnewcon.end()) {
				
				int near=0; 
				
				while (near!=vertices[*it]->links->size()) {
					present[*it]=1;
					if (connected.insert(vertices[*it]->links->l[near]).second)
						newcon.insert(vertices[*it]->links->l[near]);
					near++;
				}
				it++;
			}
		}
		
		partic.push_back(connected);
		spannet+=connected.size();
	}
	
	
	char B[100];
	cast_string_to_char(str, B);
	ofstream con_out(B);

	//cout<<"number of connected components = "<<partic.size()<<endl;
	//cout<<"dimensions"<<endl;
	
	
	int max_pcon=0;

	for (int i=0; i<partic.size(); i++) {
		
		//cout<<partic[i].size()<<"   ";
		
		
		if (partic[i].size()>=partic[max_pcon].size())
			max_pcon=i;
	}
	
	
	//cout<<endl<<endl;
	
	
	
	
	set <int>::iterator it=partic[max_pcon].begin();
	
		
	
	it=partic[max_pcon].begin();
	while (it!=partic[max_pcon].end()) {
		
		for (int j=0; j<vertices[*it]->links->size(); j++) if(vertices[*it]->id_num < vertices[vertices[*it]->links->l[j]]->id_num)
				con_out<<vertices[*it]->id_num<<"\t"<<vertices[vertices[*it]->links->l[j]]->id_num<<"\t"<<vertices[*it]->links->w[j]<<endl;
		
		it++;
	}

	return (partic.size());
}


double static_network::kin (const deque<int> & seq) {
	
	
	if(seq.size()>2*tstrength/dim) {
	
		set<int> H;
		deque_to_set(seq, H);
		return kin(H);
	
	}
	
	
	double k=0;
	for (int i=0; i<seq.size(); i++)
		k+=vertices[seq[i]]->kplus(seq);

	return k;

}




double static_network::ktot (const deque<int> &seq) {
	
	double k=0;
	for (int i=0; i<seq.size(); i++)
		k+=vertices[seq[i]]->strength;
	return k;

}


double static_network::ktot(const set <int> &s) {
	
	double k=0;
	for (set<int>::iterator it=s.begin(); it!=s.end(); it++)
		k+=vertices[*it]->strength;
	
	
	
	return k;

}


double static_network::kin(const set <int> &s) {
	
	double k=0;
	for (set<int>::iterator it=s.begin(); it!=s.end(); it++)
		k+=vertices[*it]->kplus(s);
	
	
	
	return k;

}







int static_network::draw(string file_name) {

	return draw(file_name, weighted);

}

int static_network::draw_consecutive(string file_name1, string file_name2) {
	
	return draw_consecutive(file_name1, file_name2, weighted);
}

int static_network::draw_consecutive(string file_name1, string file_name2, bool _weighted_) {
	
	
	
	
	char b[128];
	cast_string_to_char(file_name1, b);
	
	
	cout<<"drawing in file "<<b<<endl;
	ofstream graph_out(b);
	
	
	
	if (_weighted_) {
		
		for (int i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->links->size(); j++) if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num)
				graph_out<<i<<"\t"<<vertices[i]->links->l[j]<<"\t"<<vertices[i]->links->w[j]<<endl;
	
	}
	
	else {
		
		for (int i=0; i<vertices.size(); i++)
			for (int j=0; j<vertices[i]->links->size(); j++) if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num)
				graph_out<<i<<"\t"<<vertices[i]->links->l[j]<<endl;
	
	
	}
	
	char bb[128];

	cast_string_to_char(file_name2, bb);
	ofstream graph_out2(bb);
	for (int i=0; i<vertices.size(); i++)
		graph_out2<<i<<" "<<vertices[i]->id_num<<endl;

		

	return 0;

}

int static_network::draw(string file_name, bool _weighted_) {
	
	
	
		int h= file_name.size();
		
		char b[h+1];
		for (int i=0; i<h; i++)
			b[i]=file_name[i];
		b[h]='\0';
		
		
		
		ofstream graph_out(b);

		
		if (_weighted_) {
			
			for (int i=0; i<vertices.size(); i++)
				for (int j=0; j<vertices[i]->links->size(); j++) /*if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num)*/
					graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->links->l[j]]->id_num<<"\t"<<vertices[i]->links->w[j]<<endl;
		
		}
		
		else {
			
			for (int i=0; i<vertices.size(); i++)
				for (int j=0; j<vertices[i]->links->size(); j++) /*if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num)*/
					graph_out<<vertices[i]->id_num<<"\t"<<vertices[vertices[i]->links->l[j]]->id_num<<endl;
		
		
		}

	return 0;

}


void static_network::rewiring(deque<set<int> >& en) {
	
	
	
	
	deque<pair<int, int> > Es;
	
	for(int i=0; i<en.size(); i++) for(set<int>::iterator its=en[i].begin(); its!=en[i].end(); its++) if(i<*its)
		Es.push_back(make_pair(i, *its));
	
	
	
	
	
	for(int kk=0; kk<10; kk++) for(int i=0; i<Es.size(); i++) {
	
	
	
	
		int a=irand(Es.size()-1);
		int b=irand(Es.size()-1);
		
		while(true) {
		
			if(a==b)
				break;
				
			

			int a1= Es[a].first;
			int a2= Es[a].second;
		
			int b1, b2;
			
			
			if(ran4()<0.5) {
				b1= Es[b].first;
				b2= Es[b].second;
			
			} else {
			
				b1= Es[b].second;
				b2= Es[b].first;
			}

		
			//cout<<a1<<" "<<a2<<"-- "<<b1<<" "<<b2<<endl;
			
			
			if(!(en[a1].find(b1)==en[a1].end() && en[a2].find(b2)==en[a2].end() && a1!=b1 && a2!=b2))
				break;
			
			en[a1].erase(a2);
			en[a2].erase(a1);
			en[b1].erase(b2);
			en[b2].erase(b1);
			
			en[a1].insert(b1);
			en[a2].insert(b2);
			en[b1].insert(a1);
			en[b2].insert(a2);
			
			
			
			
			Es[a].first=a1;
			Es[a].second=b1;
			Es[b].first=a2;
			Es[b].second=b2;
			
		
		
			break;
		
		}
	
	
	
	
	}
	
	
	

}

void static_network::print_random_version(string file_name) {
	
	
	char b[100];
	cast_string_to_char(file_name, b);
	ofstream outt(b);
	
	deque<int> nodes;
	deque<int> degrees;
	
	for(int i=0; i<dim; i++)
		nodes.push_back(vertices[i]->id_num);
	
	for(int i=0; i<dim; i++)
		degrees.push_back(cast_int(vertices[i]->strength));


	/*
	cout<<"nodes"<<endl;
	prints(nodes);
	
	cout<<"degrees"<<endl;
	prints(degrees);
	//*/
	

	
		
	// this function is to build a network with the labels stored in nodes and the degree seq in degrees (correspondence is based on the vectorial index)
	
	
	
	// labels will be placed in the end
	deque<set<int> > en; // this is the E of the subgraph

	{
		set<int> first;
		for(int i=0; i<nodes.size(); i++) 
			en.push_back(first);
	}
	
	
	
	multimap <int, int> degree_node;
	
	for(int i=0; i<degrees.size(); i++)
		degree_node.insert(degree_node.end(), make_pair(degrees[i], i));
	
	int var=0;

	while (degree_node.size() > 0) {
		
		multimap<int, int>::iterator itlast= degree_node.end();
		itlast--;
		
		multimap <int, int>::iterator itit= itlast;
		deque <multimap<int, int>::iterator> erasenda;
		
		int inserted=0;
		
		for (int i=0; i<itlast->first; i++) {
			
			if(itit!=degree_node.begin()) {
			
				itit--;
				
				
				en[itlast->second].insert(itit->second);
				en[itit->second].insert(itlast->second);
				inserted++;
				
				erasenda.push_back(itit);				
				
			}
			
			else
				break;
		
		}
		
		
		for (int i=0; i<erasenda.size(); i++) {
			
			
			if(erasenda[i]->first>1)
				degree_node.insert(make_pair(erasenda[i]->first - 1, erasenda[i]->second));
	
			degree_node.erase(erasenda[i]);
		
		}

		
		var+= itlast->first - inserted;
		degree_node.erase(itlast);
		
	}

	
	
	// this is to randomize the subgraph -------------------------------------------------------------------
	
	rewiring(en);
	

	
	
	for(int i=0; i<en.size(); i++)
		for(set<int>::iterator it= en[i].begin(); it!=en[i].end(); it++)
			outt<<vertices[i]->id_num<<" "<<vertices[*it]->id_num<<endl;
		

	
	

}



void static_network::get_id_label (map <int, int> &a) {
	
	for (int i=0; i<dim; i++)
		a.insert(make_pair(vertices[i]->id_num, i));


}

void static_network::deque_id(deque<int> & a) {
	
	for(int i=0; i<a.size(); i++)	
		a[i]=vertices[a[i]]->id_num;


}

void static_network::print_id(const deque<int> & a, ostream & pout) {
	
	for (int i=0; i<a.size(); i++)
		pout<<vertices[a[i]]->id_num<<"\t";
	pout<<endl;


}


void static_network::print_id(const set<int> & a, ostream & pout) {
	
	for (set<int>::iterator its=a.begin(); its!=a.end(); its++)
		pout<<vertices[*its]->id_num<<"\t";
	pout<<endl;


}



void static_network::print_id(const deque<deque<int> > & a, ostream & pout) {
	
	for(int i=0; i<a.size(); i++)
		print_id(a[i], pout);
	


}

void static_network::print_id(const deque<set<int> > & a, ostream & pout) {
	
	for(int i=0; i<a.size(); i++)
		print_id(a[i], pout);
	


}



int static_network::translate(deque<deque<int> > & ten) {

	map<int, int> A;
	get_id_label(A);
	
	for(int i=0; i<ten.size(); i++) {
		for(int j=0; j<ten[i].size(); j++) {
			
			map<int, int>::iterator itf=A.find(ten[i][j]);
			if(itf==A.end()) {
				
				cerr<<"ERROR: the nodes in the communities are different from those ones in the network!"<<endl;
				//return -1;
			
			
			}
			
			ten[i][j]=itf->second;
			
		}
		
		
		sort(ten[i].begin(), ten[i].end());
		
	}
	
	return 0;


}


void static_network::print_component(string file_name, const deque<int> & group) {


	int h= file_name.size();
	char b[h+1];
	cast_string_to_char(file_name, b);
	ofstream subout(b);

	
	
	for(int i=0; i<group.size(); i++) {
		
		int nodei=group[i];
		
		for(int j=0; j<vertices[nodei]->links->size(); j++)
			subout<<vertices[nodei]->id_num<<" "<<vertices[vertices[nodei]->links->l[j]]->id_num<<" "<<vertices[nodei]->links->w[j]<<endl;
			
	}


}

void static_network::print_subgraph(string file_name, const deque<int> & group) {


	int h= file_name.size();
	char b[h+1];
	cast_string_to_char(file_name, b);
	ofstream subout(b);

	
	
	for(int i=0; i<group.size(); i++) {
		
		int nodei=group[i];
		
		for(int j=0; j<group.size(); j++) {
			
			
			double wij=vertices[nodei]->links->posweightof(group[j]).second;
			if(wij>0 && vertices[nodei]->id_num < vertices[group[j]]->id_num)
				subout<<vertices[nodei]->id_num<<" "<<vertices[group[j]]->id_num<<" "<<wij<<endl;
			
		
		
		}
		
	
	
	
	}


}

void static_network::next_shell(const set<int> & shell0, set<int> & shell1, set<int> & already) {
	
	shell1.clear();
	for(set<int>::iterator its=shell0.begin(); its!=shell0.end(); its++)
		for(int j=0; j<vertices[*its]->links->size(); j++) if(already.find(vertices[*its]->links->l[j])==already.end())
			shell1.insert(vertices[*its]->links->l[j]);
	



}







void static_network::draw_pajek_directed(map<int, double> & lab_x, map<int, double> & lab_y, string file_name, deque<deque<int> >  M, double scale_factor, 
											deque<int> & node1, deque<int> & node2, deque<double> & w12) {

	
	
	double bari_x=0;
	double bari_y=0;
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		bari_x+=itm->second;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		bari_y+=itm->second;

	bari_x/=dim;
	bari_y/=dim;
	
	
	//cout<<"bari: "<<bari_x<<" "<<bari_y<<endl;
	//cout<<"scale_factor: "<<scale_factor<<endl;
	
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		itm->second-=bari_x;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		itm->second-=bari_y;

	/*
	bari_x=0;
	bari_y=0;
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		bari_x+=itm->second;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		bari_y+=itm->second;

	bari_x/=dim;
	bari_y/=dim;
	
	cout<<"bari: "<<bari_x<<" "<<bari_y<<endl;*/
	
	
	double rescale_factor=1;
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		if(fabs(itm->second)>rescale_factor)
			rescale_factor=fabs(itm->second);
	
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		if(fabs(itm->second)>rescale_factor)
			rescale_factor=fabs(itm->second);
	
	rescale_factor*=2;
	
	rescale_factor/=scale_factor;
	
	
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		itm->second=itm->second/rescale_factor + 0.5;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		itm->second=itm->second/rescale_factor + 0.5;

	
	//cout<<"rescale_factor: "<<rescale_factor<<endl;
	//rescale_factor*=10;
	
	
	
	
	int h= file_name.size();
	char b[h+1];
	cast_string_to_char(file_name, b);
	ofstream subout(b);
	
	
	deque<string> pajek_colors;
	
	pajek_colors.push_back("GreenYellow");
	pajek_colors.push_back("Yellow");
	pajek_colors.push_back("Goldenrod");
	pajek_colors.push_back("Dandelion");
	pajek_colors.push_back("Apricot");	
	pajek_colors.push_back("Peach");
	pajek_colors.push_back("Melon");
	pajek_colors.push_back("YellowOrange");
	pajek_colors.push_back("Orange");
	pajek_colors.push_back("BurntOrange");
	pajek_colors.push_back("Bittersweet");
	pajek_colors.push_back("RedOrange");
	pajek_colors.push_back("Mahogany");
	pajek_colors.push_back("Maroon");
	pajek_colors.push_back("BrickRed");
	pajek_colors.push_back("Red");
	pajek_colors.push_back("OrangeRed");
	pajek_colors.push_back("RubinRed");
	pajek_colors.push_back("WildStrawberry");
	pajek_colors.push_back("Salmon");
	pajek_colors.push_back("CarnationPink");
	pajek_colors.push_back("Magenta");
	pajek_colors.push_back("VioletRed");
	pajek_colors.push_back("Rhodamine");
	pajek_colors.push_back("Mulberry");
	pajek_colors.push_back("RedViolet");
	pajek_colors.push_back("Fuchsia");
	pajek_colors.push_back("Lavender");
	pajek_colors.push_back("Thistle");
	pajek_colors.push_back("Orchid");
	pajek_colors.push_back("DarkOrchid");
	pajek_colors.push_back("Purple");
	pajek_colors.push_back("Plum");
	pajek_colors.push_back("Violet");
	pajek_colors.push_back("RoyalPurple");
	pajek_colors.push_back("BlueViolet");
	pajek_colors.push_back("Periwinkle");
	pajek_colors.push_back("CadetBlue");
	pajek_colors.push_back("CornflowerBlue");
	pajek_colors.push_back("MidnightBlue");
	pajek_colors.push_back("NavyBlue");
	pajek_colors.push_back("RoyalBlue");
	pajek_colors.push_back("Blue");
	pajek_colors.push_back("Cerulean");
	pajek_colors.push_back("Cyan");
	pajek_colors.push_back("ProcessBlue");
	pajek_colors.push_back("SkyBlue");
	pajek_colors.push_back("Turquoise");
	pajek_colors.push_back("TealBlue");
	pajek_colors.push_back("Aquamarine");
	pajek_colors.push_back("BlueGreen");
	pajek_colors.push_back("Emerald");
	pajek_colors.push_back("JungleGreen");
	pajek_colors.push_back("SeaGreen");
	pajek_colors.push_back("Green");
	pajek_colors.push_back("ForestGreen");
	pajek_colors.push_back("PineGreen");
	pajek_colors.push_back("LimeGreen");
	pajek_colors.push_back("YellowGreen");
	pajek_colors.push_back("SpringGreen");
	pajek_colors.push_back("OliveGreen");
	pajek_colors.push_back("RawSienna");
	pajek_colors.push_back("Sepia");
	pajek_colors.push_back("Brown");
	pajek_colors.push_back("Tan");
	
	
	pajek_colors.push_back("Gray");
	pajek_colors.push_back("Black");
	pajek_colors.push_back("White");
	
	deque<int> node_color;

	//----------------------------------------------------------------------------
	deque<int> pcolors(pajek_colors.size()-3);
	for(int i=0; i<pcolors.size(); i++)
		pcolors[i]=i;
	
	shuffle_s(pcolors);
	
	deque<set<int> > node_member;
	translate(M);
	
	set<int> first;
	for(int i=0; i<dim; i++)
		node_member.push_back(first);
	
	
	for(int i=0; i<M.size(); i++)
		for(int j=0; j<M[i].size(); j++)
			node_member[M[i][j]].insert(i);
	//printm(node_member);
	
	
	for(int i=0; i<node_member.size(); i++) {
	
		
		if(node_member[i].size()==1)
			node_color.push_back(pcolors[*node_member[i].begin()%int(pcolors.size())]);
		else if(node_member[i].size()==0)
			node_color.push_back(pajek_colors.size()-1);
		else
			node_color.push_back(pajek_colors.size()-2);
	
	}	
	//----------------------------------------------------------------------------
	
	
	subout<<"*Vertices "<<dim<<endl;
	for(int i=0; i<dim; i++)
		subout<<"   "<<i+1<<" \""<<id_of(i)<<"\" "<<lab_x[id_of(i)]<<" "<<lab_y[id_of(i)]<<" 0.5 "<<"box ic "<<pajek_colors[node_color[i]]<<endl;


	map<int, int> AA;
	get_id_label(AA);

	subout<<"*Arcs"<<endl;
	for(int i=0; i<node1.size(); i++) {
		
		subout<<AA[node1[i]]+1<<" "<<AA[node2[i]]+1;
		
		if(w12.size()>0)
			subout<<" "<<w12[i];
		
		subout<<endl;
	
	}
	
	


}















void static_network::draw_pajek(map<int, double> & lab_x, map<int, double> & lab_y, string file_name, deque<deque<int> >  M, double scale_factor) {

	
	
	double bari_x=0;
	double bari_y=0;
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		bari_x+=itm->second;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		bari_y+=itm->second;

	bari_x/=dim;
	bari_y/=dim;
	
	
	//cout<<"bari: "<<bari_x<<" "<<bari_y<<endl;
	//cout<<"scale_factor: "<<scale_factor<<endl;
	
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		itm->second-=bari_x;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		itm->second-=bari_y;

	/*
	bari_x=0;
	bari_y=0;
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		bari_x+=itm->second;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		bari_y+=itm->second;

	bari_x/=dim;
	bari_y/=dim;
	
	cout<<"bari: "<<bari_x<<" "<<bari_y<<endl;*/
	
	
	double rescale_factor=1;
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		if(fabs(itm->second)>rescale_factor)
			rescale_factor=fabs(itm->second);
	
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		if(fabs(itm->second)>rescale_factor)
			rescale_factor=fabs(itm->second);
	
	rescale_factor*=2;
	
	rescale_factor/=scale_factor;
	
	
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		itm->second=itm->second/rescale_factor + 0.5;
	for(map<int, double> :: iterator itm=lab_y.begin(); itm!=lab_y.end(); itm++)
		itm->second=itm->second/rescale_factor + 0.5;

	
	//cout<<"rescale_factor: "<<rescale_factor<<endl;
	//rescale_factor*=10;
	
	
	
	
	int h= file_name.size();
	char b[h+1];
	cast_string_to_char(file_name, b);
	ofstream subout(b);
	
	
	deque<string> pajek_colors;
	
	pajek_colors.push_back("GreenYellow");
	pajek_colors.push_back("Yellow");
	pajek_colors.push_back("Goldenrod");
	pajek_colors.push_back("Dandelion");
	pajek_colors.push_back("Apricot");	
	pajek_colors.push_back("Peach");
	pajek_colors.push_back("Melon");
	pajek_colors.push_back("YellowOrange");
	pajek_colors.push_back("Orange");
	pajek_colors.push_back("BurntOrange");
	pajek_colors.push_back("Bittersweet");
	pajek_colors.push_back("RedOrange");
	pajek_colors.push_back("Mahogany");
	pajek_colors.push_back("Maroon");
	pajek_colors.push_back("BrickRed");
	pajek_colors.push_back("Red");
	pajek_colors.push_back("OrangeRed");
	pajek_colors.push_back("RubinRed");
	pajek_colors.push_back("WildStrawberry");
	pajek_colors.push_back("Salmon");
	pajek_colors.push_back("CarnationPink");
	pajek_colors.push_back("Magenta");
	pajek_colors.push_back("VioletRed");
	pajek_colors.push_back("Rhodamine");
	pajek_colors.push_back("Mulberry");
	pajek_colors.push_back("RedViolet");
	pajek_colors.push_back("Fuchsia");
	pajek_colors.push_back("Lavender");
	pajek_colors.push_back("Thistle");
	pajek_colors.push_back("Orchid");
	pajek_colors.push_back("DarkOrchid");
	pajek_colors.push_back("Purple");
	pajek_colors.push_back("Plum");
	pajek_colors.push_back("Violet");
	pajek_colors.push_back("RoyalPurple");
	pajek_colors.push_back("BlueViolet");
	pajek_colors.push_back("Periwinkle");
	pajek_colors.push_back("CadetBlue");
	pajek_colors.push_back("CornflowerBlue");
	pajek_colors.push_back("MidnightBlue");
	pajek_colors.push_back("NavyBlue");
	pajek_colors.push_back("RoyalBlue");
	pajek_colors.push_back("Blue");
	pajek_colors.push_back("Cerulean");
	pajek_colors.push_back("Cyan");
	pajek_colors.push_back("ProcessBlue");
	pajek_colors.push_back("SkyBlue");
	pajek_colors.push_back("Turquoise");
	pajek_colors.push_back("TealBlue");
	pajek_colors.push_back("Aquamarine");
	pajek_colors.push_back("BlueGreen");
	pajek_colors.push_back("Emerald");
	pajek_colors.push_back("JungleGreen");
	pajek_colors.push_back("SeaGreen");
	pajek_colors.push_back("Green");
	pajek_colors.push_back("ForestGreen");
	pajek_colors.push_back("PineGreen");
	pajek_colors.push_back("LimeGreen");
	pajek_colors.push_back("YellowGreen");
	pajek_colors.push_back("SpringGreen");
	pajek_colors.push_back("OliveGreen");
	pajek_colors.push_back("RawSienna");
	pajek_colors.push_back("Sepia");
	pajek_colors.push_back("Brown");
	pajek_colors.push_back("Tan");
	
	
	pajek_colors.push_back("Gray");
	pajek_colors.push_back("Black");
	pajek_colors.push_back("White");
	
	deque<int> node_color;

	//----------------------------------------------------------------------------
	deque<int> pcolors(pajek_colors.size()-3);
	for(int i=0; i<pcolors.size(); i++)
		pcolors[i]=i;
	
	shuffle_s(pcolors);
	
	deque<set<int> > node_member;
	translate(M);
	
	set<int> first;
	for(int i=0; i<dim; i++)
		node_member.push_back(first);
	
	
	for(int i=0; i<M.size(); i++)
		for(int j=0; j<M[i].size(); j++)
			node_member[M[i][j]].insert(i);
	//printm(node_member);
	
	
	for(int i=0; i<node_member.size(); i++) {
	
		
		if(node_member[i].size()==1)
			node_color.push_back(pcolors[*node_member[i].begin()%int(pcolors.size())]);
		else if(node_member[i].size()==0)
			node_color.push_back(pajek_colors.size()-1);
		else
			node_color.push_back(pajek_colors.size()-2);
	
	}	
	//----------------------------------------------------------------------------
	
	
	subout<<"*Vertices "<<dim<<endl;
	for(int i=0; i<dim; i++)
		subout<<"   "<<i+1<<" \""<<id_of(i)<<"\" "<<lab_x[id_of(i)]<<" "<<lab_y[id_of(i)]<<" 0.5 "<<"box ic "<<pajek_colors[node_color[i]]<<endl;

	subout<<"*Edges"<<endl;
	for (int i=0; i<vertices.size(); i++)
		for (int j=0; j<vertices[i]->links->size(); j++) if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num)
			subout<<i+1<<" "<<vertices[i]->links->l[j]+1<<" "<<vertices[i]->links->w[j]<<endl;

	
	


}



void static_network::pajek_print_cluster(string file_name, deque<int> group, int number_of_shells) {


	int h= file_name.size();
	char b[h+1];
	cast_string_to_char(file_name, b);
	ofstream subout(b);
	
	
	deque<string> pajek_colors;
	pajek_colors.push_back("Red");
	pajek_colors.push_back("Green");
	pajek_colors.push_back("Black");
	pajek_colors.push_back("Yellow");
	pajek_colors.push_back("White");
	pajek_colors.push_back("Yellow");
	pajek_colors.push_back("Brown");
	pajek_colors.push_back("Mahogany");
	pajek_colors.push_back("Grey");
	pajek_colors.push_back("Blue");
	
	
	map<int, int> shell;
	set<int> shell0;
	set<int> already;
	deque_to_set(group, shell0);
	
	int nsh=0;
	for(set<int>::iterator its=shell0.begin(); its!=shell0.end(); its++) {
		shell[vertices[*its]->id_num]=nsh;
		already.insert(*its);
	}
	
	
	for(int i=0; i<number_of_shells; i++) {
		
		
		set<int> shell1;
		next_shell(shell0, shell1, already);
		nsh++;
		
		shell0=shell1;
		for(set<int>::iterator its=shell0.begin(); its!=shell0.end(); its++) {
			shell[vertices[*its]->id_num]=nsh;
			group.push_back(*its);
			already.insert(*its);
		}
	
	}
	
	

	
	map<int, int> node_plab;
	
	int pl=0;
		
	for(int i=0; i<group.size(); i++) {
		
		if(node_plab.find(vertices[group[i]]->id_num)==node_plab.end())
			node_plab[vertices[group[i]]->id_num]=++pl;
	
	}
	
	
	map<int, int> inverse;
	for(map<int, int>::iterator itm=node_plab.begin(); itm!=node_plab.end(); itm++)
		inverse[itm->second]=itm->first;
	
	
	subout<<"*Vertices "<<node_plab.size()<<endl;
	for(map<int, int>::iterator itm=inverse.begin(); itm!=inverse.end(); itm++)
		subout<<"   "<<itm->first<<" \""<<itm->second<<"\" box ic "<<pajek_colors[shell[itm->second]%pajek_colors.size()]<<endl;

	
	subout<<"*Edges"<<endl;
	pl=group.size();
	
	
	for(int i=0; i<group.size(); i++) {
		
		
		for(int j=0; j<vertices[group[i]]->links->size(); j++) {
			if(vertices[group[i]]->id_num < vertices[vertices[group[i]]->links->l[j]]->id_num && node_plab.find(vertices[vertices[group[i]]->links->l[j]]->id_num)!=node_plab.end())
				subout<<node_plab[vertices[group[i]]->id_num]<<" "<<node_plab[vertices[vertices[group[i]]->links->l[j]]->id_num]<<" "<<vertices[group[i]]->links->w[j]<<endl;
		}
		
	
	
	
	}
	
	char pbb[1000];
	sprintf(pbb, "unixdos %s %s.net", b, b);
	cout<<pbb<<endl;
	//int sy=system(pbb);

}



void static_network::set_subgraph(deque<int> & group, deque<deque<int> > & link_per_node, deque<deque<double> > & weights_per_node) {
	
	
	// in this function I'm not using id's... because I want to work with the same labels (don't want to translate)
	
	
	
	
	
	sort(group.begin(), group.end());

	weights_per_node.clear();
	link_per_node.clear();
	
	for(int i=0; i<group.size(); i++) {
		
		int nodei=group[i];
		
		
		deque<int> link_i;
		deque<double> weight_i;
		
		
		for(int j=0; j<vertices[nodei]->links->size(); j++) if(binary_search(group.begin(), group.end(), vertices[nodei]->links->l[j])) {
			
			link_i.push_back(vertices[nodei]->links->l[j]);
			weight_i.push_back(vertices[nodei]->links->w[j]);
			
		}
		
		
		link_per_node.push_back(link_i);
		weights_per_node.push_back(weight_i);
		
		
	}


}





void static_network::same_component(int source, set<int> & already_gone) {
	
	int node=source;
	double  average;
	deque<double>  ML;
	int step;
	
	already_gone.clear();
	already_gone.insert(node);
	
	deque<int> new_shell;
	new_shell.push_back(node);
	
	deque<pair<int, int> > distances_node;
	int shell=0;
	int reached=1;


	if(shell>=ML.size())
		ML.push_back(0);
	
	ML[shell]+=reached;
	

	propagate_distances(new_shell, already_gone, distances_node, shell, ML, reached, step);
	
	
	
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



void static_network::set_connected_components(deque<set<int> > & comps) {

	
	
	comps.clear();
	set<int> not_assigned;
	for(int i=0; i<dim; i++)
		not_assigned.insert(i);
	
	while(not_assigned.size()>0) {
	
		
		int source = *not_assigned.begin();
		
		set<int> mates;
		same_component(source, mates);
		
		for(set<int>::iterator its=mates.begin(); its!=mates.end(); its++)
			not_assigned.erase(*its);
			
		
		comps.push_back(mates);
		
	
	}
	

}

void static_network::print_connected_components(ostream & outb) {

	
	
	
	int spannet=0;
	deque <set<int> > partic;
	deque <int> present;
	present.assign(dim,0);

	
	while (spannet!=dim) {
		
		set <int> connected;
		set <int> newcon;
				

		for (int i=0; i<dim; i++)
			if (present[i]==0) {
				connected.insert(i);
				newcon.insert(i);
				present[i]=1;
				break;
			}
			
		
	
		while (newcon.size()!=0) {
			
			set <int> nnewcon=newcon;
			newcon.clear();
			set <int>::iterator it=nnewcon.begin();
			while (it!=nnewcon.end()) {
				
				int near=0; 
				
				while (near!=vertices[*it]->links->size()) {
					present[*it]=1;
					if (connected.insert(vertices[*it]->links->l[near]).second)
						newcon.insert(vertices[*it]->links->l[near]);
					near++;
				}
				it++;
			}
		}
		
		partic.push_back(connected);
		spannet+=connected.size();
	}
	
	
	for(int i=0; i<partic.size(); i++) {
		for(set<int>::iterator its=partic[i].begin(); its!=partic[i].end(); its++)
			outb<<vertices[*its]->id_num<<" ";
		
		outb<<endl;
	}
	



}





void static_network::propagate_bw(int* distances, int* weights, int source, set<int> & mates, set<int> & not_leaves, deque<int> & next_shell) {

	
	mates.insert(source);
	
	for(int i=0; i<vertices[source]->links->size(); i++) {
		
		int neigh=vertices[source]->links->l[i];
		
		
		
		if(mates.find(neigh)==mates.end()) {		// new vertex
			
			
			//cout<<"new "<<vertices[neigh]->id_num<<endl;
			distances[neigh]=distances[source]+1;
			weights[neigh]=weights[source];
			mates.insert(neigh);
			next_shell.push_back(neigh);
			not_leaves.insert(source);			
		
		
		} else if(distances[neigh]==distances[source]+1) {
			
			weights[neigh]+=weights[source];
			not_leaves.insert(source);

		}
		
	
	
	}


}






int static_network::compute_betweeness_single_source(int source, map<pair<int, int>, double> & tot_edge_bw) {
	
	
	
	// this function compute the edge betweenness of all the vertices in the component of source (only for source)
	set<int> mates;
	set<int> not_leaves;
	
	
	
	
	
	int distances[dim];
	int weights[dim];
	
	
	
	
	distances[source]=0;
	weights[source]=1;
	deque<int> present_shell;
	present_shell.push_back(source);
	
	while(true) {
		
		
		if(present_shell.empty())
			break;
		
		deque<int> next_shell;
		
		for(int i=0; i<present_shell.size(); i++)
			propagate_bw(distances, weights, present_shell[i], mates, not_leaves, next_shell);
		
		//cout<<"------------ next "<<endl;
		//print_id(next_shell, cout);

		present_shell=next_shell;
		
	}
	
	/*
	prints(distances, dim);
	prints(weights, dim);
	*/
	
	
	
	//print_id(mates, cout);
	
	deque<int> leaves;
	for(set<int>::iterator its=mates.begin(); its!=mates.end(); its++)
		if(not_leaves.find(*its)==not_leaves.end())
			leaves.push_back(*its);
	
	
	//cout<<"leaves"<<endl;
	//print_id(leaves, cout);
	
	map<pair<int, int>, double> edge_bw;	// map edge-betweenness
	for(set<int>::iterator its=mates.begin(); its!=mates.end(); its++) {
		
		for(int i=0; i<vertices[*its]->links->size(); i++) if(*its < vertices[*its]->links->l[i])
			edge_bw.insert(make_pair(make_pair(*its, vertices[*its]->links->l[i]), 0));
	
	
	}
	
	
	multimap<int, int> distance_not_leaves;
	for(set<int>::iterator its=not_leaves.begin(); its!=not_leaves.end(); its++)
		distance_not_leaves.insert(make_pair(-distances[*its], *its));
	
	
	for(deque<int>::iterator its=leaves.begin(); its!=leaves.end(); its++) {
		
		for(int i=0; i<vertices[*its]->links->size(); i++) if(distances[*its]>distances[vertices[*its]->links->l[i]]) {
			
			pair<int, int> ed;
			ed.first=min(*its, vertices[*its]->links->l[i]);
			ed.second=max(*its, vertices[*its]->links->l[i]);
			
			
			
			edge_bw[ed]=double(weights[vertices[*its]->links->l[i]])/weights[*its];
			tot_edge_bw[ed]+=double(weights[vertices[*its]->links->l[i]])/weights[*its];
			
			
		
		}
			
		
	
	}
	
	
	
	for(multimap<int, int>::iterator itm=distance_not_leaves.begin(); itm!=distance_not_leaves.end(); itm++) {
		
		
		
		//cout<<"node:::: "<<itm->second<<endl;
		
		
		double sum_of_weight=0;
		
		for(int i=0; i<vertices[itm->second]->links->size(); i++) {
			
			pair<int, int> ed;
			ed.first=min(itm->second, vertices[itm->second]->links->l[i]);
			ed.second=max(itm->second, vertices[itm->second]->links->l[i]);
			
			sum_of_weight+= edge_bw[ed];
		
		}
		
		
		for(int i=0; i<vertices[itm->second]->links->size(); i++) if(distances[itm->second]>distances[vertices[itm->second]->links->l[i]]) {
			
			pair<int, int> ed;
			ed.first=min(itm->second, vertices[itm->second]->links->l[i]);
			ed.second=max(itm->second, vertices[itm->second]->links->l[i]);
			
			edge_bw[ed]=double(weights[vertices[itm->second]->links->l[i]])/weights[itm->second]*(1 + sum_of_weight);
			tot_edge_bw[ed]+=double(weights[vertices[itm->second]->links->l[i]])/weights[itm->second]*(1 + sum_of_weight);
			
			//cout<<"pred--> "<<vertices[itm->second]->links->l[i]<<" "<<double(weights[vertices[itm->second]->links->l[i]])/weights[itm->second]*(1 + sum_of_weight)<<" "<<sum_of_weight<<endl;
			
			
		}
		
		
	
	
	}
	
	
	//cout<<"************************"<<endl;
	
	
		
	
	return 0;
	
		
}



int static_network::component_betweeness(int source, map<pair<int, int>, double> & tot_edge_bw, set<int> & mates) {		// this compute the betweenness of the edges in the component of source
	
	
	
	mates.clear();
	
	same_component(source, mates);
	
	
	for(set<int>::iterator its=mates.begin(); its!=mates.end(); its++) {
		
		for(int j=0; j<vertices[*its]->links->size(); j++) if(*its < vertices[*its]->links->l[j]) {
			
			
			pair<int, int> ed;
			ed.first=*its;
			ed.second=vertices[*its]->links->l[j];
			
			tot_edge_bw[ed]=0;			
			
			
		}
	
	}



	for(set<int>::iterator its=mates.begin(); its!=mates.end(); its++)		// this must be made n times
		compute_betweeness_single_source(*its, tot_edge_bw);								// this requires m operations
	
	
	
	
	//for(map<pair<int, int>, double>::iterator itm=tot_edge_bw.begin(); itm!=tot_edge_bw.end(); itm++)
		//cout<<vertices[itm->first.first]->id_num<<" "<<vertices[itm->first.second]->id_num<<" "<<itm->second<<endl;
	
	
	
	
	return 0;


}





int static_network::all_betweeness(map<pair<int, int>, double> & tot_edge_bw) {

	// this compute the betweenness of all the edges
	
	
	
	
	set<int> not_assigned;
	for(int i=0; i<dim; i++)
		not_assigned.insert(i);
	
	while(not_assigned.size()>0) {
	
		
		int source = *not_assigned.begin();
		set<int> mates;
		
		component_betweeness(source, tot_edge_bw, mates);		
		
		for(set<int>::iterator its=mates.begin(); its!=mates.end(); its++)
			not_assigned.erase(*its);

	
	}
	
	
	//for(map<pair<int, int>, double>::iterator itm=tot_edge_bw.begin(); itm!=tot_edge_bw.end(); itm++)
		//cout<<vertices[itm->first.first]->id_num<<" "<<vertices[itm->first.second]->id_num<<" "<<itm->second<<endl;
	

		
	return 0;


}





int static_network::set_mem_adj (deque<deque<int> > & mem_adj) {		// unweighted networks!
	
	mem_adj.clear();
	
	for (int i=0; i<dim; i++) {
		
		deque <int> first(dim);
		for (int j=0; j<dim; j++)
			first[j]=0;
		
		for (int j=0; j<vertices[i]->links->size(); j++)
			first[vertices[i]->links->l[j]]=1;
		
		mem_adj.push_back(first);
		
	
	}
	
	return 0;
}

int static_network::propagate_distances(deque<int> & new_shell, set<int> & already_gone, deque<pair<int, int> > & distances_node, int shell, deque<double> & ML, int & reached, int step) {

	shell++;
	deque<int> next_shell;

	
	
	for(int i=0; i<new_shell.size(); i++) for(int j=0; j<vertices[new_shell[i]]->links->size(); j++) {
		
		
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
		
		
		
		if(shell>=ML.size())
			ML.push_back(dim*step);
	
		reached+=next_shell.size();
		ML[shell]+=reached;

		
		
		return propagate_distances(next_shell, already_gone, distances_node, shell, ML, reached, step);
	
	
	}
	
	return 0;
	

}


int static_network::distances_from_i(int node, double & average, deque<double> & ML, int step) {
	
	
	
	set<int> already_gone;
	already_gone.insert(node);
	
	deque<int> new_shell;
	new_shell.push_back(node);
	
	deque<pair<int, int> > distances_node;
	int shell=0;
	int reached=1;


	if(shell>=ML.size())
		ML.push_back(0);
	
	ML[shell]+=reached;
	

	propagate_distances(new_shell, already_gone, distances_node, shell, ML, reached, step);
	//sort(distances_node.begin(), distances_node.end());
	
	
	//cout<<"NODE "<<id_of(node)<<endl;
	
	average=0;
	int maxd=0;
	
	
	for(int i=0; i<distances_node.size(); i++) {
		
		
		average+=distances_node[i].first;
		
		
		if(distances_node[i].first>maxd)
			maxd=distances_node[i].first;
			
		//cout<<distances_node[i].first<<" "<<id_of(distances_node[i].second)<<endl;
	
	
	}
	
	for(int i=maxd+1; i<ML.size(); i++)
		ML[i]+=dim;
	
	//prints(ML);
	
	return maxd;
	
	
}



int static_network::diameter_and_asp(double & averageall, int & diameter, deque<double> & ML) {
	
	
	
	ML.clear();
	averageall=0;
	diameter=0;
	
	for(int i=0; i<dim; i++) {
		
		double average;
		
		//int previousMLsize=ML.size();
		
		int m=distances_from_i(i, average, ML, i);
		
		/*if(ML.size()>previousMLsize) {
			
			for(int j=previousMLsize; j<ML.size(); j++)
				ML[j]+=dim*(i);
				
		
		}*/
		
		if(m>diameter)
			diameter=m;
	
		averageall+=average;
	
	}
	
	
	averageall/=(dim*(dim-1));
	
	
	for(int i=0; i<ML.size(); i++)
		ML[i]/=dim;
	
	//prints(ML);
	
	return 0;
	

}







int static_network::knn(map <int, deque<double > > & knn_hist) {
	
	
	// this knn is not erased...
	
	deque<double> knn_nodo(dim);
	for (int i=0; i<dim; i++)
		knn_nodo[i]=0;
	
	for (int i=0; i<dim; i++) {
		for (int j=0; j<vertices[i]->links->size(); j++)
			knn_nodo[i]+=vertices[vertices[i]->links->l[j]]->strength;	
	}
	
	
	
	for(int i=0; i<dim; i++) if(vertices[i]->strength>0) {
	
		
		map<int, deque<double> > ::iterator it=knn_hist.find(vertices[i]->links->size());		
		if(it==knn_hist.end()) {
				
			deque<double> f;
			knn_hist[vertices[i]->links->size()]=f;
			
		}
				
		knn_hist[vertices[i]->links->size()].push_back(knn_nodo[i]/vertices[i]->strength);	
	
	}
	
	
	return 0;
	
}




int static_network::knn(map <int, double> & knn_hist) {
	
	
	double knn_nodo[dim];
	for (int i=0; i<dim; i++)
		knn_nodo[i]=0;
	
	for (int i=0; i<dim; i++) {
		for (int j=0; j<vertices[i]->links->size(); j++)
			knn_nodo[i]+=vertices[vertices[i]->links->l[j]]->strength;	
	}
	
	knn_hist.clear();
	map <int, int> degree_hist;
	
	for (int i=0; i<dim; i++) if(vertices[i]->strength>0) {
		
		map <int, double>::iterator itf = knn_hist.find(vertices[i]->links->size());
		map <int, int>::iterator itf2 = degree_hist.find(vertices[i]->links->size());
			if (itf!=knn_hist.end()) {
			
				itf->second+=knn_nodo[i];
				itf2->second+=vertices[i]->links->size();
			
			}
			else {
				knn_hist.insert(make_pair(vertices[i]->links->size(), knn_nodo[i]));
				degree_hist.insert(make_pair(vertices[i]->links->size(), vertices[i]->links->size()));			
			}
	}
	
		
	
	
	map <int, int>::iterator it2= degree_hist.begin();
	
	for (map <int, double>::iterator it1= knn_hist.begin(); it1!=knn_hist.end(); it1++) {
		it1->second=it1->second/(it2->second);
		it2++;
		
	}
	
	
	
	
	
	/*
	double k2av=0;
	for(int i=0; i<dim; i++)
		k2av+=(vertices[i]->strength)*(vertices[i]->strength);
	 
	
	cout<<"VALUE "<<k2av/(2*tstrength)<<endl;
	*/
	
	
	
	
	return 0;
	
}




double static_network::clustering_coefficient() {

	
	double number_of_triangles=0;
	double possible=0;

	for(int i=0; i<dim; i++) {
		
		for(int j=0; j<vertices[i]->links->size(); j++) for(int k=0; k<vertices[vertices[i]->links->l[j]]->links->size(); k++)
			number_of_triangles+=min(vertices[i]->links->posweightof(vertices[vertices[i]->links->l[j]]->links->l[k]).first, 0) +1;
		
		possible=(vertices[i]->links->size())*(vertices[i]->links->size()-1.)/2.;
		
	}
	
	return number_of_triangles/possible;
	
	
	
	

}



void static_network::clustering_coefficient(map <int, deque<double> > & c_of_k) {

	
	// c_of_k is a map kin -> clu. c. and it is not cleared
	
	
	for(int i=0; i<dim; i++) {
		
		set<int> intneighs;		// neighbors of node c[i]
		for(int j=0; j<vertices[i]->links->size(); j++)
			intneighs.insert(vertices[i]->links->l[j]);
			
		
		
		
		{
			
			map<int, deque<double> > ::iterator it=c_of_k.find(intneighs.size());		
			if(it==c_of_k.end()) {
				
				deque<double> f;
				c_of_k[intneighs.size()]=f;
			
			}
				
			if(intneighs.size() > 1)
				c_of_k[intneighs.size()].push_back(kin(intneighs)/ (intneighs.size() * (intneighs.size() -1)));		
			else
				c_of_k[intneighs.size()].push_back(0);		
		}

		
	}


}



bool static_network::GN_bench(int nodes, int modules, double kout, double m_degree, deque<deque<int> > & ten) {
	
			
	  
	ten.clear();
					
	int nm;
	nm=nodes/modules;
	nodes=nm*modules;
	
	
	
	
	double pin=(m_degree - kout) / (nm-1);
	double pout= kout/(nm*(modules-1));		
	
	cout<<"pin "<<pin<<endl;
	if(modules>1)
		cout<<"pout "<<pout<<endl;
	
	
	int count=1;
	deque<int> community;
	for (double i=0; i<nodes; i++) {

		if (i>=nm*count) {
			ten.push_back(community);
			community.clear();
			count++;
		}
		
		community.push_back(cast_int(i));
		
	}
	
	ten.push_back(community);
	
	
	
	deque<deque<int> > e;
	deque<int> s1;
	
	for (double i=0; i<nodes; i++)
		e.push_back(s1);
	
	count=1;
	int edges=0;

	
	for (int i=0; i<nodes; i++) {
		
		if (i>=nm*count)
			count++;
			
		for (int j=i+1; j<nodes; j++) {
		
			
			double p0;
			
			if (j<(nm*count))
				p0=pin;
			else
				p0=pout;
			
			
			if (ran4()<=p0) {
				e[i].push_back(j);
				e[j].push_back(i);
				edges++;
			}
		
		}
	}	
	
	
	
	cout<<"------------------------------------------------------------------"<<endl;
	cout<<"network di "<<nodes<<" nodi;\t"<<edges<<" lati;\t"<<"grado medio: "<<2.*edges/nodes<<endl;
	cout<<"------------------------------------------------------------------"<<endl;
	
	
	deque<int> labels;
	for(int i=0; i<nodes; i++)
		labels.push_back(i);
			
	set_graph(e, labels);
	
	
	
	return true;
	
}



bool static_network::set_binary_random(const deque<int> & degrees) {
	
	  
	
	deque<set<int> > en;

	{
		set<int> first;
		for(int i=0; i<degrees.size(); i++) 
			en.push_back(first);
	}
	
	
	
	multimap <int, int> degree_node;
	
	for(int i=0; i<degrees.size(); i++)
		degree_node.insert(degree_node.end(), make_pair(degrees[i], i));
	

	while (degree_node.size() > 0) {
		
		multimap<int, int>::iterator itlast= degree_node.end();
		itlast--;
		
		multimap <int, int>::iterator itit= itlast;
		deque <multimap<int, int>::iterator> erasenda;
		
		int inserted=0;
		
		for (int i=0; i<itlast->first; i++) {
			
			if(itit!=degree_node.begin()) {
			
				itit--;
				
				
				en[itlast->second].insert(itit->second);
				en[itit->second].insert(itlast->second);
				inserted++;
				
				erasenda.push_back(itit);				
				
			}
			
			else
				break;
		
		}
		
		
		for (int i=0; i<erasenda.size(); i++) {
			
			
			if(erasenda[i]->first>1)
				degree_node.insert(make_pair(erasenda[i]->first - 1, erasenda[i]->second));
	
			degree_node.erase(erasenda[i]);
		
		}

		
		degree_node.erase(itlast);
		
	}

	
	
	
	rewiring(en);
	

	
	deque<deque<int> > E;
	for(int i=0; i<en.size(); i++) {
		
		deque<int> f;
		set_to_deque(en[i], f);
		E.push_back(f);
	
	}
		
		
	deque<int> labels;
	for(int i=0; i<degrees.size(); i++)
		labels.push_back(i);
			
	set_graph(E, labels);

	
	return true;

}



// it computes the integral of a power law
double integral (double a, double b) {

	
	if (fabs(a+1.)>1e-10)
		return (1./(a+1.)*pow(b, a+1.));
	
	
	else
		return (log(b));

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


bool static_network::set_random_powlaw(int num_nodes, double tau, double average_k, int max_degree) {

	  
	
	double dmin=solve_dmin(max_degree, average_k, -tau);
	if (dmin==-1)
		return false;
	
	int min_degree=int(dmin);
	
	
	double media1=integer_average(max_degree, min_degree, tau);
	double media2=integer_average(max_degree, min_degree+1, tau);
	
	if (fabs(media1-average_k)>fabs(media2-average_k))
		min_degree++;
	
		
	deque <int> degree_seq ;		//  degree sequence of the nodes
	deque <double> cumulative;
	powerlaw(max_degree, min_degree, tau, cumulative);

	for (int i=0; i<num_nodes; i++) {
		
		int nn=lower_bound(cumulative.begin(), cumulative.end(), ran4())-cumulative.begin()+min_degree;
		degree_seq.push_back(nn);

	}
	
	
	
	set_binary_random(degree_seq);
	
	return true;

}





bool static_network::set_multiple_random(const deque<int> & degrees, bool self_loops) {

	
	// set self_loops = false to avoid self_loops
	
	int number_of_nodes=0;
	int number_of_links=0;
	for (int i=0; i<degrees.size(); i++) {
		
		number_of_nodes++;
		number_of_links+=degrees[i];
	
	}
	
	
	
	deque<int> random_links(number_of_links);
	

	{
		
		
		deque<int> random_position(number_of_links);
		
		for(int i=0; i<number_of_links; i++)
			random_position[i]=i;
		
		shuffle_s(random_position);
		
		
		int label=0;
		int position=0;
		for (int i=0; i<degrees.size(); i++) {

			for (int k=0; k<degrees[i]; k++)
				random_links[random_position[position++]]=label;
					
			label++;
		
		}
	
	
	
	}
	
	
	int number_of_pairs=number_of_links/2;

	if(number_of_links%2==1)
		number_of_links--;


	//this is done to avoid self loops
	
	if(self_loops==false) {
	
		for (int pos=0; pos<number_of_links; pos+=2)	{
		
			if(random_links[pos]==random_links[pos+1])	 {		// this is a self loop
				
				bool flag=true;
				
				
				while(flag) {
					
					
					int new_pair=irand(number_of_pairs-1);
					
					
					if (random_links[2*new_pair]!=random_links[pos] && random_links[2*new_pair+1]!=random_links[pos]) {
						
						flag=false;
						random_links[pos+1]=random_links[2*new_pair+1];
						random_links[2*new_pair+1]=random_links[pos];
					
					}
				
				
				}
				
			
			}
			
		
		}
	
	}

	

	
	map < pair<int, int> , int> links_and_weights;
	
	int position=0;
	for (int i=0; i<number_of_pairs; i++) {
		
		
				
		pair<int, int> p(random_links[position], random_links[position+1]);
		
		if (random_links[position]>random_links[position+1]) {
			
			p.first=random_links[position+1];
			p.second=random_links[position];
		
		}
			
		

		position+=2;
		
		
		map < pair<int, int> , int>::iterator itf= links_and_weights.find(p);
		
		if (itf==links_and_weights.end()) {
			
			if (p.first==p.second)
				links_and_weights.insert(make_pair(p, 2));
			else
				links_and_weights.insert(make_pair(p, 1));
			
		
		}
		
		else {
			
			if (p.first==p.second)
				itf->second+=2;
			else
				itf->second++;
		
		}
	
	
	
	}
	
	
	deque<deque<int> > E;
	deque<deque<double> > Ew;
	deque<int> labels;
	
	for(int i=0; i<degrees.size(); i++) {
		
		deque<int> f;
		E.push_back(f);
		deque<double> ff;
		Ew.push_back(ff);
		labels.push_back(i);
	
	}
		
			
			
	

	for (map < pair<int, int> , int>::iterator itf= links_and_weights.begin(); itf!=links_and_weights.end(); itf++) {
		
		E[itf->first.first].push_back(itf->first.second);
		Ew[itf->first.first].push_back(itf->second);
		E[itf->first.second].push_back(itf->first.first);
		Ew[itf->first.second].push_back(itf->second);
	
	}
	
	
	set_graph(true, E, Ew, labels);
	
	
	
	return true;



}



bool static_network::set_random_powlaw_multiple(int num_nodes, double tau, double average_k, int max_degree, bool self_loops) {

	  
	
	double dmin=solve_dmin(max_degree, average_k, -tau);
	if (dmin==-1)
		return false;
	
	int min_degree=int(dmin);
	
	
	double media1=integer_average(max_degree, min_degree, tau);
	double media2=integer_average(max_degree, min_degree+1, tau);
	
	if (fabs(media1-average_k)>fabs(media2-average_k))
		min_degree++;
	
		
	deque <int> degree_seq ;		//  degree sequence of the nodes
	deque <double> cumulative;
	powerlaw(max_degree, min_degree, tau, cumulative);

	for (int i=0; i<num_nodes; i++) {
		
		int nn=lower_bound(cumulative.begin(), cumulative.end(), ran4())-cumulative.begin()+min_degree;
		degree_seq.push_back(nn);

	}
	
	
	
	set_multiple_random(degree_seq, self_loops);
	
	return true;

}



void static_network::community_netknn(deque<deque<int> > & ten) {

	
	
	// this function computes something...
	
	
	
	
	deque<int> members(dim);
	for(int i=0; i<ten.size(); i++) {
		
		for(int j=0; j<ten[i].size(); j++) {
			
			members[ten[i][j]]=i;
			
		
		}
	
	}
	
	prints(members);
	set<pair<int, int> > links;
	
	
	for(int i=0; i<dim; i++) {
		
		for(int j=0; j<vertices[i]->links->size(); j++) {
			
			int ne=vertices[i]->links->l[j];
			if(members[i]!=members[ne]) {
				
				links.insert(make_pair(members[i], members[ne]));
				
			
			
			}
		
		}
			
			
	
	}
	
	
	map<int, deque<int> > size_size;
	for(set<pair<int, int> > :: iterator it=links.begin(); it!=links.end(); it++) {
	
		
		
		int one=it->first;
		int two=it->second;
		
		if(size_size.find(ten[one].size())!=size_size.end())
			size_size[ten[one].size()].push_back(ten[two].size());
		else {
			
			deque<int> f;
			size_size[ten[one].size()]=f;
			size_size[ten[one].size()].push_back(ten[two].size());
		
		}
	
		one=it->second;
		two=it->first;
		
		if(size_size.find(ten[one].size())!=size_size.end())
			size_size[ten[one].size()].push_back(ten[two].size());
		else {
			
			deque<int> f;
			size_size[ten[one].size()]=f;
			size_size[ten[one].size()].push_back(ten[two].size());
		
		}
	
	}


	ofstream sout("size_size");
	for(map<int, deque<int> >  :: iterator it=size_size.begin(); it!=size_size.end(); it++) {
		
		sout<<it->first<<" "<<average_func(it->second)<<endl;
	
	}
	
	
	


}


void static_network::community_net(deque<deque<int> > & ten) {

	
	
	// this function computes something...
	
	
	
	
	deque<int> members(dim);
	for(int i=0; i<ten.size(); i++) {
		
		for(int j=0; j<ten[i].size(); j++) {
			
			members[ten[i][j]]=i;
			
		
		}
	
	}
	
	//prints(members);
	map<pair<int, int> , int> links;
	
	
	for(int i=0; i<dim; i++) {
		
		for(int j=0; j<vertices[i]->links->size(); j++) {
			
			int ne=vertices[i]->links->l[j];
			if(members[i]!=members[ne]) {
				
				pair<int, int> P(members[i], members[ne]);
				
				if(links.find(P)==links.end())
					links[P]=0;
				
				links[P]++;
				
			
			
			}
		
		}
			
			
	
	}
	
	
	

	ofstream sout("com_net");
	for(map<pair<int, int> , int >  :: iterator it=links.begin(); it!=links.end(); it++) {
		
		sout<<it->first.first<<" "<<it->first.second<<" "<<it->second<<endl;
	
	}
	
	
	


}



double static_network::monte_carlo_asp() {
	
	
	if(dim<1000) {
	
		deque<double> ML;
		double avt;
		int diam;
		diameter_and_asp(avt, diam, ML);
		
		return avt;
	

	}
	
	
	deque<double> asps;
				
	for(int i=0; i<1000; i++) {
		
		double average;
		
		
		deque<double> ML;
		int m=distances_from_i(irand(dim-1), average, ML, irand(dim-1));
		
		asps.push_back(average/dim);
			
			
	}
	
	
	
	return average_func(asps);
	

}


void static_network::add_isolated(int id_iso) {


	vertices.push_back(new vertex (id_iso, 0, 0));
	dim++;


}


bool mate(deque<set<int> > & node_member, int i, int j) {

	
	vector<int> v(node_member[i].size());                           
	vector<int>::iterator it;
 
   
	it=set_intersection(node_member[i].begin(), node_member[i].end(), node_member[j].begin(), node_member[j].end(), v.begin()); 
	if(int(it - v.begin())==0)
		return false;
	
	return true;
}






double static_network::draw_gnuplot(map<int, double> & lab_x, map<int, double> & lab_y, ostream & goo, bool internal, deque<deque<int> >  M, double width) {
	
	
	
	deque<set<int> > node_member;
	translate(M);
	set<int> first;
	for(int i=0; i<dim; i++)
		node_member.push_back(first);
	
	
	for(int i=0; i<M.size(); i++)
		for(int j=0; j<M[i].size(); j++)
			node_member[M[i][j]].insert(i);
	
	
	
	
		
	char b[1000];
	for (int i=0; i<vertices.size(); i++) for (int j=0; j<vertices[i]->links->size(); j++) if(vertices[i]->id_num <= vertices[vertices[i]->links->l[j]]->id_num) {
		
		int & neigh= vertices[i]->links->l[j];
		if( (mate(node_member, i, neigh)==true && internal) ||  (mate(node_member, i, neigh)==false && !internal) )  {
			sprintf(b, "set arrow from %f,%f to %f,%f nohead lw %f", lab_x[id_of(i)], lab_y[id_of(i)], lab_x[id_of(neigh)], lab_y[id_of(neigh)], width);	
			goo<<b<<endl;
		}
	
	}

	
	return 0;
	
}















#endif




