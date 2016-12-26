

#include "standard_package/standard_include.cpp"
#define log_table_pr 1e-5



class log_fact_table {



	public:
		log_fact_table() {};
		~log_fact_table(){};
		
		double log_factorial(int a) { return lnf[a]; };
		void _set_(int);
		//void set_small_tab_right_hyper(int, int);
		
		inline double log_hyper(int kin_node, int kout_g, int tm, int degree_node) {   return log_choose(kout_g, kin_node) + log_choose(tm - kout_g, degree_node - kin_node) - log_choose(tm, degree_node);   };
		inline double hyper(int kin_node, int kout_g, int tm, int degree_node) { return max(0., exp(log_hyper(kin_node, kout_g, tm, degree_node)));  };
		inline double binom(int x, int N, double p) { return exp(  log_choose(N, x) + x*log(p) + (N-x) * log(1-p)    ); };
		inline double log_choose(int tm, int degree_node) {  return lnf[tm] - lnf[tm - degree_node] - lnf[degree_node];  };
		
		double cum_binomial_right(int x, int N, double prob);
		double cum_binomial_left(int x, int N, double prob);
		
		inline double log_symmetric_eq(int k1, int k2, int H, int x) {	return -x * lnf[2] - lnf[k1-x] - lnf[k2 -x] - lnf[x+H] -lnf[x];  };
		double slow_symmetric_eq(int k1, int k2, int H, int x);
		//vector<vector<vector<vector<double> > > > small_rh;		/*********/// small_rh[tm][kout][k][kin]		tm>=kout>=k>=kin
		//double right_cum_symmetric_eq(int k1, int k2, int H, int x);		
		double fast_right_cum_symmetric_eq(int k1, int k2, int H, int x, int mode, int tm);
		double right_cumulative_function(int k1, int k2, int k3, int x);
	
	
	
		
	private:
	
	
		vector<double> lnf;

		double loghyper1(int tm, int degree_node, int kout_g);
		double hyper2(int tm, int degree_node, int kout_g, int x, double constlog);
		inline double sym_ratio(int & k1, int & k2, int & H, double i) { return 0.5 * (k1 - i +1) / ((i+H)*i)   * (k2 - i +1); };

		double cum_hyper_right(int kin_node, int kout_g, int tm, int degree_node);
		double cum_hyper_left(int kin_node, int kout_g, int tm, int degree_node);


};






void log_fact_table::_set_(int size) {

	cout<<"allocating "<<size<<" factorials..."<<endl;
	lnf.clear();
	
	lnf.reserve(size+1);
	
	double f=0;
	lnf.push_back(0);
	
	for(int i=1; i<=size; i++) {
		
		f+=log(i);
		lnf.push_back(f);
	}
	
	cout<<"done"<<endl;
	//prints(lnf);
	


}




double log_fact_table::cum_hyper_right(int kin_node, int kout_g, int tm, int degree_node) {
	
	//cout<<"kin_node... "<<kin_node<<" "<<kout_g<<" "<<tm<<" "<<degree_node<<endl; 
	// this is bigger  or equal p(x >= kin_node)   *** EQUAL ***
	
	if(kin_node>min(degree_node, kout_g))
		return 0;
	
	
	
	if(tm - kout_g - degree_node + kin_node <=0)
		return 1;
	
	if(kin_node<=0)
		return 1;
	
	
	if(kin_node<double(kout_g+1)/double(tm +2)*double(degree_node+1))
		return (1. - cum_hyper_left(kin_node, kout_g, tm, degree_node));
	
	
	
	
	int x=kin_node;
	double pzero= hyper(x, kout_g, tm, degree_node);
	
	
	//*
	if(pzero<=1e-40)
		return 0;
	//*/
	
	
	
	
	double ga= tm - kout_g - degree_node;
	int kout_g_p = kout_g +1;
	double degree_node_p = degree_node +1;
	double z_zero= 1.;
	double sum= z_zero;
	
	
	
	while(true) {
	
		
		++x;
		
		z_zero *= double(kout_g_p -x) / (x * (ga + x)) * (degree_node_p - x);

		
		if(z_zero < log_table_pr * sum)
			break;
		
		if(pzero * sum>1)
			return pzero;
		
		sum+=z_zero;	
	}
	
	
	return pzero * sum;
	
}



double log_fact_table::cum_hyper_left(int kin_node, int kout_g, int tm, int degree_node) {
	
	
	// this is strictly less  p(x < kin_node)   *** NOT EQUAL ***
	//cout<<kin_node<<" node: "<<degree_node<<" group: "<<tm<<" "<<degree_node<<endl;
	
	
	if(kin_node<=0)
		return 0;

	if(tm - kout_g - degree_node + kin_node <=0)
		return 0;	
	
	if(kin_node>min(degree_node, kout_g))
		return 1;	

	
	
	if(kin_node>double(kout_g+1)/double(tm +2)*double(degree_node+1))
		return (1. - cum_hyper_right(kin_node, kout_g, tm, degree_node));
	
	
		
	
	int x=kin_node-1;
	double pzero= hyper(x, kout_g, tm, degree_node);
	
	//cout<<"pzero: "<<pzero<<" "<<log_hyper(x, kout_g, tm, degree_node)<<" gsl: "<<(gsl_ran_hypergeometric_pdf(x, kout_g, tm - kout_g,  degree_node))<<endl;
	
	
	
	//*
	if(pzero<=1e-40)
		return 0;
	//*/
	
	
	
	
	double ga= tm - kout_g - degree_node;
	int kout_g_p = kout_g +1;
	double degree_node_p = degree_node +1;
	double z_zero= 1.;
	double sum= z_zero;
	
	//cout<<"pzero "<<pzero<<" "<<z_zero<<" "<<kin_node<<endl;
	
	while(true) {
	
		
		
		
		z_zero *= (ga + x) / ((degree_node_p - x) *(kout_g_p -x))  * x;
		--x;
		//cout<<"zzero sum "<<z_zero<<" "<<sum<<" "<<(ga + x)<<endl;
		
		if(z_zero< log_table_pr *sum)
			break;
		
		if(pzero * sum>1)
			return pzero;
		
		sum+=z_zero;	
	}
	
	
	return pzero * sum;
	
}


double log_fact_table::cum_binomial_right(int x, int N, double prob) {

	// this is bigger  or equal p(x >= kin_node)   *** EQUAL ***
	
	//cout<<"x "<<x<<" N "<<N <<"  prob "<<prob<<endl;
	
	
	if(x<=0) 
		return 1;
	
	if(x>N)
		return 0;
	
	
	if(prob-1> - 1e-11)
		return 1;
	
	if(x<N*prob)
		return 1-cum_binomial_left(x, N, prob);
	
	
	double pzero= binom(x, N, prob);
	
	
	if(pzero<=1e-40)
		return 0;
	
	
	double z_zero= 1.;
	double sum= z_zero;
	
	
	while(true) {
	
		
		
		
		z_zero *=  prob * double(N-x) / ((x+1)*(1-prob));
		x++;
		//cout<<"zzero sum "<<z_zero<<" "<<sum<<" "<<endl;
		
		if(z_zero< log_table_pr * sum)
			break;
		
		sum+=z_zero;	
	}
	
	
	return pzero * sum;




}





double log_fact_table::cum_binomial_left(int x, int N, double prob) {

	// this is less strictly p(x < kin_node)   *** NOT EQUAL ***
	
	
	if(x<=0)
		return 0;
	
	if(x>N)
		return 1;
	
	
	if(prob<1e-11)
		return 1;
	
	if(x>N*prob)
		return 1-cum_binomial_right(x, N, prob);
	
	--x;
	double pzero= binom(x, N, prob);
	
	
	if(pzero<=1e-40)
		return 0;
	
	
	double z_zero= 1.;
	double sum= z_zero;
	
	while(true) {
	
		
		
		--x;
		z_zero *=  (1-prob) * double(x+1) / ((N-x) *prob);
		
		//cout<<"zzero sum "<<z_zero<<" "<<sum<<" "<<(ga + x)<<endl;
		
		if(z_zero< log_table_pr * sum)
			break;
		
		sum+=z_zero;	
	}
	
	
	return pzero * sum;
}






double log_fact_table::slow_symmetric_eq(int k1, int k2, int H, int x) {
	
	// k1, k2 and k3 are the three colors
	
	//cout<<"k3: "<<k3<<endl;
	

	int l1=max(0, -H);
	int l2=min(k1, k2);
	
	//cout<<"l1: "<<l1<<" l2: "<<l2<<endl;
	
	
	if(x<l1)
		return 0;
	
	if(x>l2)
		return 0;
	
	
	double p=0;
	for(int ix=l1; ix<=l2; ++ix) {
		
		//cout<<ix<<" "<<(log_symmetric_eq(k1, k2, H, ix))<<endl;
		p+=exp(log_symmetric_eq(k1, k2, H, ix));
	
	}
	
	//cout<<"p: "<<p<<endl;
	
	
	
	return exp(log_symmetric_eq(k1, k2, H, x))/p;

}




inline double log_fact_table::fast_right_cum_symmetric_eq(int k1, int k2, int H, int x, int mode, int tm) {
	
	// I want k1 to be the smaller between the two
		
	
	
	//cout<<"k1 "<<k1<<" "<<k2<<" "<<H<<" "<<x<<" "<<mode<<" "<<2*H+k1+k2<<endl;
	
	
	if(k1>k2)
		return fast_right_cum_symmetric_eq(k2, k1, H, x, mode, tm);
	

	
	double ri=1;
	double q1=0;
	double q2=0;
	double ratio;
	
	if(x==mode)
		++q2;
	else
		++q1;

	int l1=max(0, -H);
	
	double ii=mode-1;
	
	
	while(ii>=l1) {
		
		ratio = sym_ratio(k1, k2, H, ii+1);
		ri /= ratio;
		
		q1+=ri;
		if(q1> 1e280)
			return cum_hyper_right(x, k2, tm, k1);
		

		
		
		if(ri<log_table_pr*q1)
			break;
		
		--ii;
		

	}
	
	
	
	/*double cum1=exp(log_symmetric_eq(k1, k2, H, l1));
	double lg0=log_symmetric_eq(k1, k2, H, l1);
	cout<<"x: "<<l1<<" "<<exp(log_symmetric_eq(k1, k2, H, l1))<<" "<<exp(lg0)<<endl;*/
	ri=1;
	ii=mode+1;
	//for(double i=mode+1; i<x; i++) 
	while(ii<x) {
		
		
		ratio = sym_ratio(k1, k2, H, ii);
		ri *= ratio;
		
		q1+=ri;
		if(q1> 1e280)
			return cum_hyper_right(x, k2, tm, k1);
		
		if(ri<log_table_pr*q1)
			break;
		//cout<<ii<<" "<<ratio<<" "<<ri/q1<<" b"<<endl;;
		++ii;
		
		//cout<<"dx-->: "<<ii<<" "<<exp(log_symmetric_eq(k1, k2, H, ii))<<" "<<exp(lg0) * ri<<" "<<sym_ratio(k1, k2, H, ii+1)<<endl;

	}
	
	
	
	ii=max(x, mode+1);
	ri=exp(log_symmetric_eq(k1, k2, H, cast_int(ii-1)) - log_symmetric_eq(k1, k2, H, mode));
	
	//for(double i=max(x, mode+1); i<=k1; i++) 
	
	while(ii<=k1) {
		
		ratio = sym_ratio(k1, k2, H, ii);
		ri *= ratio;
		q2+=ri;
		if(q2> 1e280)
			return cum_hyper_right(x, k2, tm, k1);
		//cout<<ii<<" "<<ratio<<" "<<ri/q2<<" c "<<x<<" "<<q2<<endl;;
		++ii;
		if(ri<log_table_pr*q2)
			break;

		
		//cout<<"ddx-->: "<<i<<" "<<exp(log_symmetric_eq(k1, k2, H, i))<<" "<<exp(lg0) * ri<<" "<<sym_ratio(k1, k2, H, i+1)<<endl;

	}


	
	/* cout<<"fast q12: "<<q1<<" "<<q2<<endl;*/
	
	return max(q2/(q1+q2), 1e-100);
	

}









