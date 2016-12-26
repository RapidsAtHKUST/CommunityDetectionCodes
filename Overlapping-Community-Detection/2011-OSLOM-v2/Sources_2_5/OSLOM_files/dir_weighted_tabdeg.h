

#include "./standard_package/standard_include.cpp"

typedef multimap<double, pair<int, double> > cup_data_struct;
# define sqrt_two 1.41421356237
# define num_up_to 5
# define bisection_precision 1e-2





double compare_r_variables(double a, double b, double c, double d) {
	
	
	// r1 \in (a,b)
	// r2 \in (c,d)
	// compute the probability p(r2<r1)
	
	//cout<<a<<" "<<b<<" "<<c<<" "<<d<<endl;
	
	
	if (c<a)
		return (1-compare_r_variables(c, d, a, b));

	if(c>b)
		return 0;
	
	
	if(d>b)
		return 0.5*(b-c)*(b-c)/((d-c)*(b-a));
	else
		return (b -0.5*(d+c))/(b-a);
	
		

}








double right_error_function(double x) {
	
	return 0.5*erfc(x/sqrt_two);
}


double log_together(double minus_log_total, int number) {
	
	
	
	
	if(number<11) {
	
		double fa=1;
		double zum=1;
		
		for(int i=1; i<number; i++) {
		
			fa *= minus_log_total / i;
			zum+=fa;
		
		}
        
        
        return max(zum*exp(-minus_log_total), 1e-100);

	}
	
	
	
	double mu=number;
	return max(right_error_function((minus_log_total-mu)/sqrt(mu)), 1e-100);

}





double fitted_exponent(int N) {



	double l=log(double(N));

	
	if(N>100)
		return 4.2*l -8.5;

	if(N>30)
		return 3.5*l -5.5;

	if(N>7)
		return 2.5*l -2;

	if(N>1)
		return 1.3*l +0.1;

	
	return 1;



}




inline double order_statistics_left_cumulative(int N, int pos, double x) {
	
	
	// the routine computes the probality c_pos=  p(X_pos <= x)
	// N is the total number of variables, pos is from 1 to N. N is the smallest.
	
	
	return LOG_TABLE.cum_binomial_right(N-pos+1, N, x);

}




double inverse_order_statistics(int sample_dim, int pos, const double  & zerof, double lo, double hi) {
	
	
	
	// anyway it's possible that I will tabulated this stuff for N small like sample_dim < 10000 which can already help a lot.
	// moreover, i should not use bisection but something better
	// finally there is the gaussian approx which could be studied more...
	//file: /Users/admin/Desktop/ambiente/right_binomial_cum/approx.cpp
	
	
	//double zerof= - log(1-threshold)/fitted_exponent(sample_dim);
	
	
	
	
	double mid =  (hi+lo)/2;
	
	while ((mid != lo) && (mid != hi) ) {
		
		double fmid=order_statistics_left_cumulative(sample_dim, pos, mid);
		
		if(fabs(fmid-zerof) < bisection_precision*zerof)
			break;
		
		
		if ((fmid-zerof) <= 0)
			lo = mid;
		else
			hi = mid;
		
		mid = (hi+lo)/2;
		
	}
	
	
	return mid;
	
}




inline double pron_min_exp(int N, double xi) {


	// this should return the probability that the minimum of the quantiles p(c_min<=xi)
	//cout<<"-> "<<fitted_exponent(N)<<endl;
	
	return 1 - exp(-fitted_exponent(N)*xi);


}




inline double compute_probability_to_stop(const double & a, const double & b, const double & critical_xi, int Nstar, int pos) {


	/*	this routine is to compute the bootstrap probaility that the node with extremes a and b will be below threshold 
		when I already know that the average is below critical_xi */
	
	if(order_statistics_left_cumulative(Nstar, pos, b)<=critical_xi)
		return 1;
	
	return ( inverse_order_statistics(Nstar, pos, critical_xi, (a+b)*0.5, b)  -a )/(b-a);

				


}

/*
void small_simulation(int pos_first, int pos_last, double & A_average, double & B_average, int equivalents, int Nstar, const double & critical_xi) {

	int counter=0;
	for(int i=0; i<100000; i++) {
	
	
		deque<double> A;
		for(int j=0; j<equivalents; j++)
			A.push_back(A_average+ran4()*(B_average-A_average));
	
		sort(A.begin(), A.end());
		//prints(A);
		if(A[A.size()-2]<0.000341573)
			counter++;
	
	
	}

	cout<<"counter: "<<counter<<endl;





}*/


bool equivalent_check(int pos_first, int pos_last, double & A_average, double & B_average, int equivalents, int Nstar, const double & critical_xi) {

	// returns true is the test was passed
	
	/*small_simulation(pos_first, pos_last, A_average, B_average, equivalents, Nstar, critical_xi);
	cout<<pos_first<<" "<<pos_last<<" A, B "<<A_average<<" "<<B_average<<" "<<equivalents<<" "<<Nstar<<" "<<critical_xi<<endl;*/
	
	int pos=pos_first;
	double cr_previous=A_average;
	
	
	for(int i=equivalents; i>=1; i--) {		
		
		// loop which starts from the best node
		
		//cout<<"i... "<<i<<endl;
		
		if(order_statistics_left_cumulative(Nstar, pos, cr_previous)<=critical_xi) {
						
			if(order_statistics_left_cumulative(Nstar, pos, B_average)<=critical_xi)
				return true;
			
			
			double cr=inverse_order_statistics(Nstar, pos, critical_xi, cr_previous, B_average);
			//cout<<i<<" cr: "<<cr<<" "<<order_statistics_left_cumulative(equivalents, i, (cr-A_average)/(B_average-A_average))<<endl;
			//cout<<" this,  "<<order_statistics_left_cumulative(Nstar, pos, cr)<<" "<<Nstar<<" "<<pos<<endl;
			
			if(order_statistics_left_cumulative(equivalents, i, (cr-A_average)/(B_average-A_average))>0.5)
				return true;
				
			cr_previous=cr;
		}
		
		--pos;
		
	}


	return false;
}



bool equivalent_check_gather(cup_data_struct & a, int & until, const double & probability_a, const double & probability_b, int Nstar, const double & critical_xi) {

	int nodes_added=-1;
	cup_data_struct :: iterator itl=a.begin();

	double A_average=0;
	double B_average=0;
	int pos_first=-1;
	int pos_last=-1;
	
	while(itl!=a.end()) {
	
		if(nodes_added==until)
			break;
		
		++nodes_added;
		
		if(compare_r_variables(probability_a, probability_b, itl->first - itl->second.second, itl->first + itl->second.second)>paras.equivalence_parameter) {
		
			if(pos_first==-1)
				pos_first=Nstar-nodes_added;
			
			A_average+=itl->first - itl->second.second;
			B_average+=itl->first + itl->second.second;
			pos_last=Nstar-nodes_added;			
			//cout<<"pos_first: "<<pos_first<<" ... "<<pos_last<<endl;
		}				
		++itl;
	}
	
	
	int equivalents=pos_first-pos_last +1;
	A_average/=equivalents;
	B_average/=equivalents;
	
	if(equivalents==1)
		return true;
	
	
	if(equivalent_check(pos_first, pos_last, A_average, B_average, equivalents, Nstar, critical_xi)==true) {
		//cout<<"check passed"<<endl;
		return true;
	}
	else {
		until=-1;
		//cout<<"check not passed"<<endl;
		return false;
	}


}




inline double hyper_table(int kin_node, int kout_g, int tm, int degree_node) {
	return LOG_TABLE.hyper(kin_node, kout_g, tm, degree_node);
}


inline double compute_topologic_and_bootstrap_interval(int kin_node_out, int kout_g_in, int tmout,  int degree_node_out, double & boot_interval) {
	
	
	
	double b2out=LOG_TABLE.right_cumulative_function(degree_node_out, kout_g_in, tmout, kin_node_out+1);
	
	boot_interval = (0.5 + 1e-6*(ran4()-0.5)) * (hyper_table(kin_node_out, kout_g_in, tmout, degree_node_out));
	double topologic_out =  b2out + boot_interval;
	
	
	//cout<<"compute_topologic_and_bootstrap_interval: "<<kin_node_out<<" "<<kout_g_in<<" "<<tmout<<" "<<degree_node_out<<" "<<boot_interval<<endl;
	
	if(topologic_out<=1e-100)
		topologic_out=1e-100;
	
	if(1.-topologic_out<boot_interval)
		boot_interval=1.-topologic_out;
	if(topologic_out<boot_interval)
		boot_interval=topologic_out;
	
	return topologic_out;
}








double compute_global_fitness(int kin_node_in, int kout_g_in, int kin_node_out, int kout_g_out, int tmin, int tmout, int degree_node_in, 
										int degree_node_out, double minus_log_total_in, double minus_log_total_out, int number_of_neighs, int Nstar, double & boot_interval) {

	
	/* so, it must be completely true that kin_node is referred to the node and not to the module */
	
	if(paras.weighted==false) {
		
		if(degree_node_in==0)
			return compute_topologic_and_bootstrap_interval(kin_node_out, kout_g_in, tmout, degree_node_out, boot_interval);
		
		if(degree_node_out==0)
			return compute_topologic_and_bootstrap_interval(kin_node_in, kout_g_out, tmin, degree_node_in, boot_interval);
		
		double boot_interval_out;
		double topologic_out= compute_topologic_and_bootstrap_interval(kin_node_out, kout_g_in, tmout, degree_node_out, boot_interval_out);
		double boot_interval_in;
		double topologic_in= compute_topologic_and_bootstrap_interval(kin_node_in, kout_g_out, tmin, degree_node_in, boot_interval_in);
		
		
		double _s_= -log(topologic_in * topologic_out);
		boot_interval= _s_ *(topologic_in * boot_interval_out + topologic_out * boot_interval_in);
		
		double global_score= log_together(_s_ , 2);
		
		if(1.-global_score<boot_interval)
			boot_interval=1.-global_score;
		if(global_score<boot_interval)
			boot_interval=global_score;

		return global_score;

	}
	
	/************************************** if it is weighted *********************************************/
	
	double mu=  (number_of_neighs +1.)  / (Nstar+1.);
	
	if(degree_node_in==0) {
		
		double topologic_out= compute_topologic_and_bootstrap_interval(kin_node_out, kout_g_in, tmout, degree_node_out, boot_interval);
		double weight_part_out= log_together(minus_log_total_out, kin_node_out);
		
		topologic_out /= mu;
		boot_interval /= mu;
		if(topologic_out>1)
			topologic_out=1;

		
		double _s_= -log(topologic_out) -log(weight_part_out);
		boot_interval *= weight_part_out * _s_;
		
		double global_score= log_together(_s_ , 2);
		
		if(1.-global_score<boot_interval)
			boot_interval=1.-global_score;
		if(global_score<boot_interval)
			boot_interval=global_score;

		
		return global_score;
		
	}
	
	if(degree_node_out==0) {
		
		double topologic_in= compute_topologic_and_bootstrap_interval(kin_node_in, kout_g_out, tmin, degree_node_in, boot_interval);
		double weight_part_in= log_together(minus_log_total_in, kin_node_in);
		
		topologic_in /= mu;
		boot_interval /= mu;
		
		if(topologic_in>1)
			topologic_in=1;
		
		double _s_= -log(topologic_in) -log(weight_part_in);
		boot_interval *= weight_part_in * _s_;
		
		double global_score= log_together(_s_ , 2);
		
		if(1.-global_score<boot_interval)
			boot_interval=1.-global_score;
		if(global_score<boot_interval)
			boot_interval=global_score;

		
		return global_score;
		
	}
	
	
	
	double boot_interval_out;
	double topologic_out= compute_topologic_and_bootstrap_interval(kin_node_out, kout_g_in, tmout, degree_node_out, boot_interval_out);
	double weight_part_out= log_together(minus_log_total_out, kin_node_out);
	topologic_out /= mu;
	boot_interval_out /= mu;
	if(topologic_out>1)
		topologic_out=1;



	double boot_interval_in;
	double topologic_in= compute_topologic_and_bootstrap_interval(kin_node_in, kout_g_out, tmin, degree_node_in, boot_interval_in);
	double weight_part_in= log_together(minus_log_total_in, kin_node_in);
	topologic_in /= mu;
	boot_interval_in /= mu;
	if(topologic_in>1)
		topologic_in=1;


	
	double _s_= -log(topologic_out) -log(weight_part_out) -log(topologic_in) -log(weight_part_in);
	boot_interval= 0.16666667 * _s_ * _s_ *_s_ *(topologic_in * boot_interval_out + topologic_out * boot_interval_in) * weight_part_in * weight_part_out;
		
	double global_score= log_together(_s_ , 4);
		
	if(1.-global_score<boot_interval)
		boot_interval=1.-global_score;
	if(global_score<boot_interval)
		boot_interval=global_score;

		
	return global_score;

}




inline double compute_topologic_step(int kin_node_out, int kout_g_in, int tmout,  int degree_node_out, const double & _step_) {

	
	//cout<<"_step_ "<<_step_<<endl;
	double topologic_out =  LOG_TABLE.right_cumulative_function(degree_node_out, kout_g_in, tmout, kin_node_out+1) + _step_ * (hyper_table(kin_node_out, kout_g_in, tmout, degree_node_out));
	
	//cout<<"topologic_out: "<<topologic_out<<" "<<(hyper_table(kin_node_out, kout_g_in, tmout, degree_node_out))<<endl;
	
	if(topologic_out<=1e-100)
		topologic_out=1e-100;
	
		
	return topologic_out;
}





double compute_global_fitness_step(int kin_node_in, int kout_g_in, int kin_node_out, int kout_g_out, int tmin, int tmout, int degree_node_in, 
										int degree_node_out, double minus_log_total_in, double minus_log_total_out, int number_of_neighs, int Nstar, const double _step_) {

	
	/* so, it must be completely true that kin_node is referred to the node and not to the module */
	
	if(paras.weighted==false) {
		
		if(degree_node_in==0)
			return compute_topologic_step(kin_node_out, kout_g_in, tmout, degree_node_out, _step_);
		
		if(degree_node_out==0)
			return compute_topologic_step(kin_node_in, kout_g_out, tmin, degree_node_in, _step_);
		
		double topologic_out= compute_topologic_step(kin_node_out, kout_g_in, tmout, degree_node_out, _step_);
		double topologic_in= compute_topologic_step(kin_node_in, kout_g_out, tmin, degree_node_in, _step_);
		
		
		double _s_= -log(topologic_in * topologic_out);
		
		return log_together(_s_ , 2);

	}
	
	/************************************** if it is weighted *********************************************/
	
	double mu=  (number_of_neighs +1.)  / (Nstar+1.);
	
	if(degree_node_in==0) {
		
		double topologic_out= compute_topologic_step(kin_node_out, kout_g_in, tmout, degree_node_out, _step_);
		double weight_part_out= log_together(minus_log_total_out, kin_node_out);
		
		topologic_out /= mu;
		if(topologic_out>1)
			topologic_out=1;

		
		
		double _s_= -log(topologic_out) -log(weight_part_out);
		
		return  log_together(_s_ , 2);
	}
	
	if(degree_node_out==0) {
		
		double topologic_in= compute_topologic_step(kin_node_in, kout_g_out, tmin, degree_node_in, _step_);
		double weight_part_in= log_together(minus_log_total_in, kin_node_in);
		
		topologic_in /= mu;
		if(topologic_in>1)
			topologic_in=1;


		double _s_= -log(topologic_in) -log(weight_part_in);
				
		return log_together(_s_ , 2);
		
	}
	
	
	
	double topologic_out= compute_topologic_step(kin_node_out, kout_g_in, tmout, degree_node_out, _step_);
	double weight_part_out= log_together(minus_log_total_out, kin_node_out);
	topologic_out /= mu;
	if(topologic_out>1)
		topologic_out=1;


	double topologic_in= compute_topologic_step(kin_node_in, kout_g_out, tmin, degree_node_in, _step_);
	
	//cout<<" minus_log_total_in "<<minus_log_total_in<<" "<<minus_log_total_out<<endl;
	double weight_part_in= log_together(minus_log_total_in, kin_node_in);
	topologic_in /= mu;
	if(topologic_in>1)
		topologic_in=1;

	
	/*cout<<":: "<<weight_part_out<<endl;
	cout<<"::: "<<topologic_out<<endl;
	cout<<":: "<<weight_part_in<<endl;
	cout<<"::: "<<topologic_in<<endl;*/
	
	double _s_= -log(topologic_out) -log(weight_part_out) -log(topologic_in) -log(weight_part_in);
		
	return log_together(_s_ , 4);


}

	



inline double compute_global_fitness_ofive(int kin_node_in, int kout_g_in, int kin_node_out, int kout_g_out, int tmin, int tmout, int degree_node_in, 
										int degree_node_out, double minus_log_total_in, double minus_log_total_out, int number_of_neighs, int Nstar) {

	return compute_global_fitness_step(kin_node_in, kout_g_in, kin_node_out, kout_g_out, tmin, tmout, degree_node_in, 
										degree_node_out, minus_log_total_in, minus_log_total_out, number_of_neighs, Nstar, 0.5);

}


inline double compute_global_fitness_randomized(int kin_node_in, int kout_g_in, int kin_node_out, int kout_g_out, int tmin, int tmout, int degree_node_in, 
										int degree_node_out, double minus_log_total_in, double minus_log_total_out, int number_of_neighs, int Nstar) {
	
	
	//cout<<"kin_node_in "<<kin_node_in<<" "<<kout_g_in<<" "<<kin_node_in<<" "<<kout_g_out<<" "<<tmin<<" "<<tmout<<endl;
	
	return compute_global_fitness_step(kin_node_in, kout_g_in, kin_node_out, kout_g_out, tmin, tmout, degree_node_in, 
										degree_node_out, minus_log_total_in, minus_log_total_out, number_of_neighs, Nstar, ran4());

}





double compute_global_fitness_randomized_short(int kin_node, int kout_g, int tm, int degree_node, double minus_log_total) {
	
	// this function is used in try_to_assign_homeless. 
	// the usual problem is that we don't know the number of neighbors of the module.
	// this could be taken in account with some more thinking...
	
	double b2=LOG_TABLE.right_cumulative_function(degree_node, kout_g, tm, kin_node+1);
	
	
	double topologic=  b2 + 0.5 * (hyper_table(kin_node, kout_g, tm,  degree_node));
	
	if(paras.weighted==false)
		return max(topologic, 1e-100);

	double weight_part= log_together(minus_log_total, kin_node);
	
	if(topologic<=1e-100 || weight_part<=1e-100)
		return 1e-100;
	
	return min(topologic, weight_part);


}







class facts {




	public:
	
		
		facts(int in_id, int in_od, double in_iw, double in_ow, multimap<double, int>::iterator c, int ide, int ode)  {
			
			
			fitness_iterator=c; 
			
			internal_indegree=in_id; 
			minus_log_total_wrin=in_iw; 
			
			internal_outdegree=in_od;
			minus_log_total_wrout=in_ow; 

			indegree=ide;
			outdegree=ode;
		
		
		
		};
		
		
		~facts(){};
		
		
		int indegree;
		int outdegree;
		
		int internal_indegree;						
		int internal_outdegree;
		
		
		double minus_log_total_wrin;								// wr is the right part of the exponential for the weights, this is the sum over the internal stubs of that
		double minus_log_total_wrout;								// wr is the right part of the exponential for the weights, this is the sum over the internal stubs of that
		multimap<double, int>::iterator fitness_iterator;
		
		
};




class weighted_tabdeg {


	public:
				
		weighted_tabdeg(){};
		~weighted_tabdeg(){};
		
		void _set_(weighted_tabdeg &);
		
		
		void clear();
		void edinsert(int a, int kpin, int kpout, int ktin, int ktout, double mtlwin, double mtlwout, double fit);		// this function inserts element a (or edit it if it was already inserted)
		bool erase(int a);
		void set_deque(deque<int> & );


		int size() {return lab_facts.size();};
		
		
		void print_nodes(ostream &);
		bool update_group(int a, int delta_deg_in, int delta_deg_out, double delta_win, double delta_wout, int nstar, int nn, int kout_g_in, int kout_g_out, int tmin, int tmout, int ktin, int ktout, deque<int> & to_be_erased);
		bool update_neighs(int a, int delta_deg_in, int delta_deg_out, double delta_win, double delta_wout, int nstar, int kout_g_in, int kout_g_out, int tmin, int tmout, int ktin, int ktout);
		void set_and_update_group(int nstar, int nn, int kout_g_out, int tmout, int kout_g_in, int tmin, weighted_tabdeg & one);
		void set_and_update_neighs(int nstar, int nn, int kout_g_out, int tmout, int kout_g_in, int tmin, weighted_tabdeg & one);

		int best_node(int & lab, double & best_fitness, int kout_g_in, int kout_g_out, int Nstar, int nneighs, int tmin, int tmout);
		int worst_node(int & lab, double & worst_fitness, int kout_g_in, int kout_g_out, int Nstar, int nneighs, int tmin, int tmout);
		bool is_internal(int a);
		
		
		map<int, facts> lab_facts;													// maps the label into the facts
		multimap<double, int> fitness_lab;											// maps the fitness into the label  (this can be optimized)
	
	
	

};


void weighted_tabdeg::clear() {

	lab_facts.clear();
	fitness_lab.clear();


}



void weighted_tabdeg::edinsert(int a, int kpin, int kpout, int ktin, int ktout, double mtlwin, double mtlwout, double fit) {		// this function inserts element a (or edit it if it was already inserted)
	
	
	erase(a);
	
	multimap<double, int>::iterator	fiit = fitness_lab.insert(make_pair(fit, a));
	facts F(kpin, kpout, mtlwin, mtlwout, fiit, ktin, ktout);
	
	
	
	lab_facts.insert(make_pair(a, F));
	

}


bool weighted_tabdeg::erase(int a) {		// this function erases element a if exists (and returns true)
	
	
	
	map<int, facts>::iterator itm= lab_facts.find(a);
	if(itm!=lab_facts.end()) {
		
		
		fitness_lab.erase(itm->second.fitness_iterator);
		lab_facts.erase(itm);
		
		return true;
		
	}
	
	return false;
	
}



bool weighted_tabdeg::is_internal(int a) {
	
	
	map<int, facts>::iterator itm= lab_facts.find(a);
	if(itm==lab_facts.end())
		return false;
	
	
	return true;
	
}




void weighted_tabdeg::set_deque(deque<int> & vv) {
	
	vv.clear();
	
	for(map<int, facts>::iterator itm= lab_facts.begin(); itm!=lab_facts.end(); itm++)
		vv.push_back(itm->first);

}


void weighted_tabdeg::print_nodes(ostream & outb) {
	
	
	cout<<"printing nodes:.. (lab intk mtlw fitness degree) "<<size()<<endl;
	
	for(map<int, facts>::iterator itm= lab_facts.begin(); itm!=lab_facts.end(); itm++)
		cout<<itm->first<<" "<<"indegrees(in - out):: "<<itm->second.internal_indegree<<" "<<itm->second.internal_outdegree<<" weights "<<
		itm->second.minus_log_total_wrin<<" "<<itm->second.minus_log_total_wrout<<" "<<(itm->second.fitness_iterator)->first<<" "<<itm->second.indegree<<endl;
	
	
	
}


int weighted_tabdeg::worst_node(int & lab, double & worst_fitness, int kout_g_in, int kout_g_out, int Nstar, int nneighs, int tmin, int tmout) {
	
	
	
	
	
	
	lab=-1;
	worst_fitness=-1;
	
	multimap<double, int>:: iterator bit= fitness_lab.end();
	if(bit==fitness_lab.begin())
		return -1;
	
	
	int stopper=0;
	while(bit!=fitness_lab.begin()) {
		
		bit--;
		map<int, facts> :: iterator itm= lab_facts.find(bit->second);
		
		
		
		
		int kout_g_out_prime= kout_g_out + itm->second.internal_indegree + itm->second.internal_outdegree - itm->second.outdegree;
		int kout_g_in_prime= kout_g_in + itm->second.internal_indegree + itm->second.internal_outdegree - itm->second.indegree;
		
		/*cout<<"Q "<<kout_g_in_prime<<endl;
		cout<<"QQ "<<kout_g_out_prime<<endl;
		cout<<"itm->second.internal_indegree "<<itm->second.internal_indegree<<endl;
		cout<<"itm->second.internal_outdegree "<<itm->second.internal_outdegree<<endl;
		cout<<"tmin: "<<tmin + itm->second.indegree<<" "<< tmout + itm->second.outdegree<<endl;*/
		
		
		double F= compute_global_fitness_randomized(itm->second.internal_indegree, kout_g_in_prime,	itm->second.internal_outdegree, kout_g_out_prime,
													tmin + itm->second.indegree, tmout + itm->second.outdegree, itm->second.indegree, itm->second.outdegree,
													itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, nneighs+1, Nstar+1);
		
		
		
		if(F>worst_fitness) {
			
			worst_fitness= F;
			lab=itm->first;
		}
		
		stopper++;
		if(stopper==num_up_to)
			break;
	
	}
	
	
	
	
	
	
	return 0;

}








int weighted_tabdeg::best_node(int & lab, double & best_fitness, int kout_g_in, int kout_g_out, int Nstar, int nneighs, int tmin, int tmout) {
	
	
	
	
	
	lab=-1;
	best_fitness=1;
	
	multimap<double, int>:: iterator bit= fitness_lab.begin();
	if(bit==fitness_lab.end()) {
		return -1;
	}
	
	int stopper=0;
	while(bit!=fitness_lab.end()) {
		
		map<int, facts> :: iterator itm= lab_facts.find(bit->second);
		
		double F= compute_global_fitness_randomized(itm->second.internal_indegree, kout_g_in, itm->second.internal_outdegree, kout_g_out, tmin, tmout, itm->second.indegree, itm->second.outdegree,
													itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, nneighs, Nstar);
		
		if(F<best_fitness) {
			
			best_fitness= F;
			lab=itm->first;
		}
		
		stopper++;
		if(stopper==num_up_to)
			break;
		
		bit++;


	}
	
	
	
	
	
	
	return 0;

}






void weighted_tabdeg::_set_(weighted_tabdeg & one)  {

	
	clear();
	for(map<int, facts>::iterator itm= one.lab_facts.begin(); itm!=one.lab_facts.end(); itm++)
		edinsert(itm->first, itm->second.internal_indegree, itm->second.internal_outdegree, itm->second.indegree, itm->second.outdegree, 
		itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, (itm->second.fitness_iterator)->first);
		
	
	
	

}



bool weighted_tabdeg::update_group(int a, int delta_degree_out, int delta_degree_in, double delta_mtlwout, double delta_mtlwin, 
											int nstar, int nn, int kout_g_in, int kout_g_out, int tmin, int tmout, int ktin, int ktout, deque<int> & to_be_erased) {

	
	
	// this function is to change the internal degree and mtlw of a certain node (to insert it or erase if necessary)
	
	
	map<int, facts>::iterator itm= lab_facts.find(a);
	if(itm==lab_facts.end())
		return false;
	
	/*cout<<"-->> "<<itm->second.minus_log_total_wrin<<" "<<delta_mtlwin<<endl;
	cout<<"-->> "<<itm->second.minus_log_total_wrout<<" "<<delta_mtlwout<<endl;
	cout<<"-->> "<<itm->second.internal_indegree<<" "<<delta_degree_in<<endl;
	cout<<"-->> "<<itm->second.internal_outdegree<<" "<<delta_degree_out<<endl;*/
	
	
	itm->second.minus_log_total_wrin+=delta_mtlwin;
	itm->second.minus_log_total_wrout+=delta_mtlwout;
	
	itm->second.internal_indegree+=delta_degree_in;
	itm->second.internal_outdegree+=delta_degree_out;
	
	
	if( (itm->second.internal_indegree + itm->second.internal_outdegree)==0 && size()>1) {
		to_be_erased.push_back(a);
		return true;
	}
	
	
	
	
	//cout<<"UPdating... group "<<a<<" delta_deg_in "<<delta_degree_in<<" "<<endl;
	
	
	

	int kout_g_out_prime= kout_g_out + itm->second.internal_indegree + itm->second.internal_outdegree - itm->second.outdegree;
	int kout_g_in_prime= kout_g_in + itm->second.internal_indegree + itm->second.internal_outdegree - itm->second.indegree;
		
	//cout<<"cin"<<endl;
	double fit= compute_global_fitness_ofive(itm->second.internal_indegree, kout_g_in_prime,	itm->second.internal_outdegree, kout_g_out_prime,
													tmin + itm->second.indegree, tmout + itm->second.outdegree, itm->second.indegree, itm->second.outdegree,
													itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, nn+1, nstar+1);
	

	fitness_lab.erase(itm->second.fitness_iterator);
	multimap<double, int>::iterator	fiit = fitness_lab.insert(make_pair(fit, a));
	itm->second.fitness_iterator=fiit;
	

	return true;


}







bool weighted_tabdeg::update_neighs(int a, int delta_deg_out, int delta_deg_in, double delta_wout, double delta_win, int nstar, int kout_g_in, int kout_g_out, int tmin, int tmout, int ktin, int ktout) {
	
	
	
	// this function is to change the internal degree and mtlw of a certain node (to insert it or erase if necessary)
	
	//cout<<"UPdating... neighs delta_win "<<delta_win<<" ... "<<delta_wout<<endl;

	map<int, facts>::iterator itm= lab_facts.find(a);
	if(itm==lab_facts.end()) {
		
		edinsert(a, 0, 0, ktin, ktout, 0, 0, 1);
		itm= lab_facts.find(a);
	}
	
	
	itm->second.internal_indegree+=delta_deg_in;
	itm->second.internal_outdegree+=delta_deg_out;


	if((itm->second.internal_indegree + itm->second.internal_outdegree)==0) {
		//cout<<"erased from neigh update "<<a<<endl;
		erase(a);
		return true;
	}
	
	itm->second.minus_log_total_wrin+=delta_win;
	itm->second.minus_log_total_wrout+=delta_wout;
	
	
	
	//cout<<"sei"<<endl;
	double fit= compute_global_fitness_ofive(itm->second.internal_indegree, kout_g_in,	itm->second.internal_outdegree, kout_g_out,
													tmin, tmout, itm->second.indegree, itm->second.outdegree,
													itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, size(), nstar);

	
	
	fitness_lab.erase(itm->second.fitness_iterator);
	multimap<double, int>::iterator	fiit = fitness_lab.insert(make_pair(fit, a));
	itm->second.fitness_iterator=fiit;
	
	
	return true;
	
	
}



void weighted_tabdeg::set_and_update_group(int nstar, int nn, int kout_g_out, int tmout, int kout_g_in, int tmin, weighted_tabdeg & one) {

	
	
	/*this function is to set and update the fitnesses of all the nodes in cgroup*/
	
	clear();
	for(map<int, facts>::iterator itm= one.lab_facts.begin(); itm!=one.lab_facts.end(); itm++) {
				
		int kout_g_out_prime= kout_g_out + itm->second.internal_indegree + itm->second.internal_outdegree - itm->second.outdegree;
		int kout_g_in_prime= kout_g_in + itm->second.internal_indegree + itm->second.internal_outdegree - itm->second.indegree;
		
		double fit= compute_global_fitness_ofive(itm->second.internal_indegree, kout_g_in_prime, itm->second.internal_outdegree, kout_g_out_prime,
													tmin + itm->second.indegree, tmout + itm->second.outdegree, itm->second.indegree, itm->second.outdegree,
													itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, nn+1, nstar+1);
				
		edinsert(itm->first, itm->second.internal_indegree, itm->second.internal_outdegree, itm->second.indegree, itm->second.outdegree, 
				 itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, fit);
	}


}



void weighted_tabdeg::set_and_update_neighs(int nstar, int nn, int kout_g_out, int tmout, int kout_g_in, int tmin, weighted_tabdeg & one) {

	
	/*this function is to set and update the fitnesses of all the nodes in neighs*/
	
	clear();
	for(map<int, facts>::iterator itm= one.lab_facts.begin(); itm!=one.lab_facts.end(); itm++) {
		
		double fit = compute_global_fitness_ofive(itm->second.internal_indegree, kout_g_in,	itm->second.internal_outdegree, kout_g_out,
													tmin, tmout, itm->second.indegree, itm->second.outdegree,
													itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, nn, nstar);
		edinsert(itm->first, itm->second.internal_indegree, itm->second.internal_outdegree, itm->second.indegree, itm->second.outdegree, 
				 itm->second.minus_log_total_wrin, itm->second.minus_log_total_wrout, fit);	
	}


}






