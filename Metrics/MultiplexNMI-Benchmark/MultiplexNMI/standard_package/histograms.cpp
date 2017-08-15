


#if !defined(HISTOGRAMS_INCLUDED)
#define HISTOGRAMS_INCLUDED




int intlog_binning(deque<int> c, int number_of_bins, deque<double> & Xs, deque<double> & Ys, deque<double> & var) {

	// this function is to make a log_histogram along the x axis and to compute y-errors
	
	//prints(c);
	
	Xs.clear();
	Ys.clear();
	var.clear();
	
	deque<int> d;
	for(int i=0; i<int(c.size()); i++) if (c[i]>0)
		d.push_back(c[i]);
	
	c.clear();
	c=d;
	sort(c.begin(), c.end());
	
	int max=c[c.size()-1];
	int min=c[0];
	
	
	
	
	deque <double> bins;
	double step=log(min);
	if (max==min)
		max++;
	
	double bin=(log(max)-log(min))/number_of_bins;		// bin width
	
	

	while (step<=log(max)+4*bin) {
		
		//cout<<"step: "<<(exp(step))<<endl;
		
		bins.push_back(exp(step));
		step+=bin;
	}
	
	//cout<<"bins"<<endl;
	//prints(bins);
	
	deque<int> hist;
	deque<double> hist2;
	int index=0;
	
	for (int i=0; i<int(c.size()); i++) {
		
		while(c[i]-bins[index]>-1e-6) {
			
			
			index++;
			hist.push_back(0);
			hist2.push_back(0);

		}
		
		
		//cout<<index<<" "<<c[i]<<" "<<bins[index]<<" "<<endl;
				
				
		hist[index-1]++;
		hist2[index-1]+=double(c[i]);
		
	}
	
	
	deque<int> integers;
	index=0;
	for(int i=min; i<bins[bins.size()-1]-1; i++) {
		
		while(i-bins[index]>-1e-6) {
			
			
			index++;
			integers.push_back(0);
		}
		
		//cout<<i<<" "<<index<<" "<<integers[index-1]<<" ********"<<endl;
		integers[index-1]++;
	}
	
	for (int i=0; i<int(hist.size()); i++) {
		
		
		
		

		
		if(hist[i]>0) {
		
			Xs.push_back(hist2[i]/hist[i]);
			double y=double(hist[i])/(c.size()*integers[i]);
			Ys.push_back(y);
			
			//cout<<h1<<" "<<h2<<" "<<y<<" "<<(hist2[i]/hist[i])<<" ***"<<endl;
			
			var.push_back(double(hist[i])/(c.size()*c.size()*integers[i]*integers[i]));
			
			//cout<<"-> "<<hist[i]<<" "<<double(hist[i])/(c.size()*c.size()*integers[i]*integers[i])<<endl;
			
		
		}
	
	
	}
	
	
	return 0;

}



void intlog_binning(deque<int> c, int number_of_bins, string file) {
	
	
	DD Xs;
	DD Ys;
	DD var;
	
	char b[file.size()+1];
	cast_string_to_char(file, b);
	ofstream pout(b);
	
	
	intlog_binning(c, number_of_bins, Xs, Ys, var);
	RANGE_loop(i, Xs) {
		pout<<Xs[i]<<" "<<Ys[i]<<" "<<var[i]<<endl;
	}
	
}



template <typename type>
int xybinning(deque<type> &c, deque<type> &d, int number_of_bins, deque<double> & xs, deque<double> & ys, deque<double> & var, deque<int> & nums) {		
	
	
	// so, this function takes two datasets (c and d) and gathers the data in bin, takes xs and ys as the average in each bin, var is the variance of the y average 
	// the difference with the same stuff called not_norm_histogram is that the other one averages x with y weights.
	
	xs.clear();
	ys.clear();
	var.clear();
	nums.clear();
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
			
	if (max==min)
		max+=1e-3;
	
	
	
	deque<deque<double> > hist_x;		// x values in the bin
	deque<deque<double> >  hist_y;		// y values in the bin
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width
	
	deque<double> f;
	while (step<=max+2*bin) {
	
		hist_x.push_back(f);			
		hist_y.push_back(f);			
		step+=bin;
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>=min && data<=max) {
			
			
			
			int index=int((data-min)/bin);		
			//cout<<data<<" "<<exp(data)<<" "<<index<<endl;
				
			hist_x[index].push_back(double(c[i]));
			hist_y[index].push_back(double(d[i]));
		
		}
		
	}
	
	for (int i=0; i<int(hist_x.size())-1; i++) {
		
		
		double x=average_func(hist_x[i]);
		double y=average_func(hist_y[i]);
		
		//cout<<x<<" "<<exp(x)<<" "<<y<<endl;
		
		if (hist_y[i].size()>0) {
			xs.push_back(x);
			ys.push_back(y);
			var.push_back(variance_func(hist_y[i]));
			nums.push_back(hist_y[i].size());
			//cout<<x<<" "<<exp(x)<<" "<<y<<" "<<(hist_y[i].size())<<" "<<variance_func(hist_y[i])<<endl;			
		}
	}
	
	
	for(UI i=0; i<var.size(); i++)
		if(var[i]<1e-8)
			var[i]=1e-8;
	
	return 0;

}






template <typename type>
int xybinning(deque<type> &c, deque<type> &d, int number_of_bins, deque<double> & xs, deque<double> & ys, deque<double> & var) {

	
	deque<int> nums;
	return xybinning(c, d, number_of_bins, xs, ys, var, nums);

}



template <typename type>
int xybinning_and_plot(deque<type> &c, deque<type> &d, int number_of_bins, ostream & pout) {
	
	deque<double> xs;
	deque<double> ys; 
	deque<double> var;
	deque<int> nums;
	xybinning(c, d, number_of_bins, xs, ys, var, nums);
	RANGE_loop(i, xs) {
		pout<<xs[i]<<" "<<ys[i]<<" "<<sqrt(var[i])<<endl;
	}
	return 0;
}



void compute_quantiles(double q, deque<double> & y, deque<double> & qs) {


		int qv=cast_int((1-q)/2 * y.size());
		if(qv<0)
			qv=0;
		if(qv>=int(y.size()))
			qv=y.size()-1;
		
		qs.push_back(y[qv]);
		
		
		qv=cast_int((1+q)/2 * y.size());
		if(qv<0)
			qv=0;
		if(qv>=int(y.size()))
			qv=y.size()-1;
		
		qs.push_back(y[qv]);



}


template <typename type>
int xybinning_quantiles(deque<type> &c, deque<type> &d, int number_of_bins, deque<double> & xs, deque<double> & ys, deque<double> & var, deque<int> & nums, deque<deque<double> > & Mq, double qa, double qb) {		
	
	
	// so, this function takes two datasets (c and d) and gathers the data in bin, takes xs and ys as the average in each bin, var is the variance of the y average 
	// the difference with the same stuff called not_norm_histogram is that the other one averages x with y weights.
	
	xs.clear();
	ys.clear();
	var.clear();
	nums.clear();
	Mq.clear();
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
			
	if (max==min)
		max+=1e-3;
	
	
	
	deque<deque<double> > hist_x;		// x values in the bin
	deque<deque<double> >  hist_y;		// y values in the bin
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width
	
	deque<double> f;
	while (step<=max+2*bin) {
	
		hist_x.push_back(f);			
		hist_y.push_back(f);			
		step+=bin;
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>=min && data<=max) {
			
			
			
			int index=int((data-min)/bin);		
			//cout<<data<<" "<<exp(data)<<" "<<index<<endl;
				
			hist_x[index].push_back(double(c[i]));
			hist_y[index].push_back(double(d[i]));
		
		}
		
	}
	
	for (int i=0; i<hist_x.size()-1; i++) {
		
		
		
				
		double x=average_func(hist_x[i]);
		double y=average_func(hist_y[i]);
		
		
		
		//cout<<x<<" "<<exp(x)<<" "<<y<<endl;
		
		if (hist_y[i].size()>0) {
			xs.push_back(x);
			ys.push_back(y);
			var.push_back(variance_func(hist_y[i])/double(hist_y[i].size()));
			nums.push_back(hist_y[i].size());
			sort(hist_y[i].begin(), hist_y[i].end());
			
			deque<double> qs;
			compute_quantiles(qa, hist_y[i], qs);
			compute_quantiles(qb, hist_y[i], qs);
			
			Mq.push_back(qs);
			//cout<<x<<" "<<exp(x)<<" "<<y<<" "<<(hist_y[i].size())<<" "<<variance_func(hist_y[i])<<endl;

			
		}
		
		
	
	}
	
	
		
	
	for(int i=0; i<var.size(); i++)
		if(var[i]<1e-8)
			var[i]=1e-8;
	
	



	return 0;

}

template <typename type>
int log_histogram(deque<type> &c, ostream & out, int number_of_bins) {		// c is the set od data, min is the lower bound, max is the upper one
	
	
	
	deque <type> d;
	for(int i=0; i<int(c.size()); i++) if (c[i]>0)
		d.push_back(c[i]);
	
	c.clear();
	c=d;
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	
	deque <int> hist;
	deque <double> hist2;
	deque <double> bins;
	double step=log(min);
	if (max==min)
		max++;
	
	double bin=(log(max)-log(min))/number_of_bins;		// bin width
	
		

	while (step<=log(max)+2*bin) {
		
		
		bins.push_back(exp(step));
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
	}
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		int index=bins.size()-1;
		for (int j=0; j<int(bins.size())-1; j++) if(	(fabs(double(c[i])-bins[j])<1e-7) || (	double(c[i])>bins[j]	&&	double(c[i])<bins[j+1]	)	) { 
		// this could be done in a more efficient way
			
			index=j;
			break;
		
		}
		
		//cout<<hist[index]<<" "<<index<<endl;
		
				
		hist[index]++;
		hist2[index]+=double(c[i]);
		
	}
	
	
	
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		double h1= bins[i];
		double h2= bins[i+1];
		
		
		double x=hist2[i]/hist[i];
		double y=double(hist[i])/(c.size()*(h2-h1));
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;		
		
	
	
	}
	
	
	
	return 0;

}





void print_quantiles(ostream & pout, DD & css, DD & pvalues) {
	
	// print the quantiles in pvalues
	
	sort(css.begin(), css.end());
	RANGE_loop(i, pvalues) {
		
		int index=max(0, cast_int(css.size()*(pvalues[i])));
		if(index==int(css.size()))
			--index;
		if(index<0) {
			cerr<<"error: css size is "<<css.size()<<endl;
		}
		pout<<css[index]<<" ";
	}
	pout<<endl;
	
}



int log_histogram_quantiles(deque<double> &c, deque<double> &c2, ostream & out, int number_of_bins) {		
	
	// prints to out  the average, 50% quantiles and 10% and 90%
	
	
	// c is the set of data, min is the lower bound, max is the upper one
	// must be c[i]>0
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	deque <int> hist0;
	deque <double> hist;
	deque <double> hist2;
	deque <double> bins;
	map<int, DD> values_in_bins;
	
	double step=log(min);
	if (max==min)
		max++;
	
	double bin=(log(max)-log(min))/number_of_bins;		// bin width
	
	
	
	while (step<=log(max)+2*bin) {
		
		
		bins.push_back(exp(step));
		hist0.push_back(0);			
		hist.push_back(0);			
		hist2.push_back(0);
		DD voidd;
		values_in_bins[hist2.size()-1]=voidd;

		step+=bin;
	}
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		int index=bins.size()-1;
		for (int j=0; j<int(bins.size())-1; j++) if(	(fabs(double(c[i])-bins[j])<1e-7) || (	double(c[i])>bins[j]	&&	double(c[i])<bins[j+1]	)	) { 
			// this could be done in a more efficient way
			
			index=j;
			break;
			
		}
		
		//cout<<hist[index]<<" "<<index<<endl;
		
		
		hist0[index]++;
		hist[index]+=c2[i];
		hist2[index]+=double(c[i]);
		values_in_bins[index].push_back(c2[i]);
		
	}
	
	DD pvalues;
	pvalues.push_back(0.5);
	pvalues.push_back(0.1);
	pvalues.push_back(0.25);
	pvalues.push_back(0.75);
	pvalues.push_back(0.9);
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		double x=hist2[i]/hist0[i];
		double y=double(hist[i])/(hist0[i]);
		if (fabs(y)>1e-10) {
			out<<x<<"\t"<<y<<" ";
			print_quantiles(out, values_in_bins[i], pvalues);
			cout<<"check: "<<average_func(values_in_bins[i])-y<<endl;
		} else {
			cout<<"y value skipped: "<<y<<endl;
		}
	}
	
	return 0;
}







int log_histogram(deque<double> &c, deque<double> &c2, ostream & out, int number_of_bins) {		// c is the set of data, min is the lower bound, max is the upper one
	
	
	// must be c[i]>0
	
	
		
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	deque <int> hist0;
	deque <double> hist;
	deque <double> hist2;
	deque <double> bins;
	double step=log(min);
	if (max==min)
		max++;
	
	double bin=(log(max)-log(min))/number_of_bins;		// bin width
	
		

	while (step<=log(max)+2*bin) {
		
		
		bins.push_back(exp(step));
		hist0.push_back(0);			
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
	}
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		int index=bins.size()-1;
		for (int j=0; j<int(bins.size())-1; j++) if(	(fabs(double(c[i])-bins[j])<1e-7) || (	double(c[i])>bins[j]	&&	double(c[i])<bins[j+1]	)	) { 
		// this could be done in a more efficient way
			
			index=j;
			break;
		
		}
		
		//cout<<hist[index]<<" "<<index<<endl;
		
				
		hist0[index]++;
		hist[index]+=c2[i];
		hist2[index]+=double(c[i]);
		
	}
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		double x=hist2[i]/hist0[i];
		double y=double(hist[i])/(hist0[i]);
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;
	}
		
	return 0;
}






template <typename type>
int histogram (vector <type> &c, ostream & out, int number_of_bins, double b1, double b2) {		

	// this should be OK
	// c is the set of data, b1 is the lower bound, b2 is the upper one (if they are equal, default limits are used)
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
	if (b1!=b2) {
		
		min=b1;
		max=b2;
	
	}
		
	if (max==min)
		max+=1e-3;
	
	
	
	deque <int> hist;
	deque <double> hist2;
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width

	while (step<=max+2*bin) {
	
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>min && data<=max) {
			
			int index=int((data-min)/bin);		
			
				
			hist[index]++;
			hist2[index]+=double(c[i]);
		
		}
		
	}
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		
		
				
		double x=hist2[i]/hist[i];
		double y=double(hist[i])/(c.size()*bin);
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;
		
	
	}
	
	
	
			
	return 0;

}





template <typename type>
int not_norm_histogram_correlated (deque<type> &c, deque<type> &d, ostream & out, int number_of_bins, double b1, double b2) {		
	
	
	// c is the x axis, d the y, b1 is the lower bound, b2 is the upper one (if they are equal, default limits are used)
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
	if (b1!=b2) {
		
		min=b1;
		max=b2;
	
	}
		
	if (max==min)
		max+=1e-3;
	
	
	
	deque <int> hist;			// frequency in the bin
	deque <double> hist_x;		// x sum in the bin
	deque <double> hist_y;		// y sum in the bin
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width

	while (step<=max+2*bin) {
	
		hist.push_back(0);			
		hist_x.push_back(0);			
		hist_y.push_back(0);			
		step+=bin;
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>min && data<=max) {
			
			int index=int((data-min)/bin);		
			
				
			hist[index]++;
			hist_x[index]+=double(c[i]);
			hist_y[index]+=double(d[i]);
		
		}
		
	}
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		
		
				
		double x=hist_x[i]/hist[i];
		double y=hist_y[i]/hist[i];;
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;
		
	
	}
	
	
	
			
	return 0;

}




template <typename type>
int histogram (deque <type> &c, ostream & out, int number_of_bins, double b1, double b2) {		

	// this should be OK
	// c is the set of data, b1 is the lower bound, b2 is the upper one (if they are equal, default limits are used)
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
	if (b1!=b2) {
		
		min=b1;
		max=b2;
	
	}
		
	if (max==min)
		max+=1e-3;
	
	
	
	deque <int> hist;
	deque <double> hist2;
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width

	while (step<=max+2*bin) {
	
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>min && data<=max) {
			
			int index=int((data-min)/bin);		
			
				
			hist[index]++;
			hist2[index]+=double(c[i]);
		
		}
		
	}
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		
		
				
		double x=hist2[i]/hist[i];
		double y=double(hist[i])/(c.size()*bin);
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;
		
	
	}
	
	
	
			
	return 0;

}

//ofstream pout("hist.dat");
//not_norm_histogram(c, pout, 10, 0, 0);


template <typename type>
int not_norm_histogram (vector<type> &c, ostream & out, int number_of_bins, double b1, double b2) {		

	// this should be OK
	// c is the set of data, b1 is the lower bound, b2 is the upper one (if they are equal, default limits are used)
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
	if (b1!=b2) {
		
		min=b1;
		max=b2;
	
	}
		
	if (max==min)
		max+=1e-3;
	
	
	
	deque <int> hist;
	deque <double> hist2;
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width

	while (step<=max+2*bin) {
	
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>min && data<=max) {
			
			int index=int((data-min)/bin);		
			
				
			hist[index]++;
			hist2[index]+=double(c[i]);
		
		}
		
	}
	
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		
		
				
		double x=hist2[i]/hist[i];
		double y=double(hist[i])/(c.size());
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;
		
	
	}
	
	
	
			
	return 0;

}



template <typename type>
int not_norm_histogram (deque<type> &c, ostream & out, int number_of_bins, double b1, double b2) {		

	// this should be OK
	// c is the set of data, b1 is the lower bound, b2 is the upper one (if they are equal, default limits are used)
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
	if (b1!=b2) {
		
		min=b1;
		max=b2;
	
	}
		
	if (max==min)
		max+=1e-3;
	
	
	
	deque <int> hist;
	deque <double> hist2;
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width

	while (step<=max+2*bin) {
	
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>min && data<=max) {
			
			int index=int((data-min)/bin);		
			
				
			hist[index]++;
			hist2[index]+=double(c[i]);
		
		}
		
	}
	
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		double x=hist2[i]/hist[i];
		double y=double(hist[i])/(c.size());
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<"\t"<<sqrt(hist[i])/c.size()<<endl;
		
	}
	
	return 0;

}


int histogram (deque<double> &c, deque<double> &c2, ostream & out, int number_of_bins, double b1, double b2) {		

	// this should be OK
	// c is the set of data, b1 is the lower bound, b2 is the upper one (if they are equal, default limits are used)
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
	if (b1!=b2) {
		
		min=b1;
		max=b2;
	
	}
		
	if (max==min)
		max+=1e-3;
	
	
	
	deque <int> hist0;
	deque <double> hist;
	deque <double> hist2;
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width

	while (step<=max+2*bin) {
	
		hist0.push_back(0);			
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
		
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>min && data<=max) {
			
			int index=int((data-min)/bin);		
			
				
			hist0[index]++;
			hist[index]+=c2[i];
			hist2[index]+=double(c[i]*c2[i]);
		
		}
		
	}
	
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		
		
				
		double x=hist2[i]/hist[i];
		double y=double(hist[i]/hist0[i]/bin);
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;
		
	
	}
	
	
	
			
	return 0;

}


int not_norm_histogram (deque<double> &c, deque<double> &c2, ostream & out, int number_of_bins, double b1, double b2) {		

	// this should be OK
	// c is the set of data, b1 is the lower bound, b2 is the upper one (if they are equal, default limits are used)
	
	
	
	double min=double(c[0]);
	double max=double(c[0]);
	
	for (int i=0; i<int(c.size()); i++) {
		
		if (min>double(c[i]))
			min=double(c[i]);
		
		if (max<double(c[i]))
			max=double(c[i]);
		
	}
	
	
	
	min-=1e-6;
	max+=1e-6;
	
	
	
	if (b1!=b2) {
		
		min=b1;
		max=b2;
	
	}
		
	if (max==min)
		max+=1e-3;
	
	
	
	deque <int> hist0;
	deque <double> hist;
	deque <double> hist2;
		
	double step=min;
	double bin=(max-min)/number_of_bins;		// bin width

	while (step<=max+2*bin) {
	
		hist0.push_back(0);			
		hist.push_back(0);			
		hist2.push_back(0);			
		step+=bin;
		
	}
	

	
		
	
	
	for (int i=0; i<int(c.size()); i++) {
		
		
		
		double data=double(c[i]);
		
		if (data>min && data<=max) {
			
			int index=int((data-min)/bin);		
			
				
			hist0[index]++;
			hist[index]+=c2[i];
			hist2[index]+=double(c[i]*c2[i]);
		
		}
		
	}
	
	
	for (int i=0; i<int(hist.size())-1; i++) {
		
		
		
				
		double x=hist2[i]/hist[i];
		double y=double(hist[i]/hist0[i]);
		
		if (fabs(y)>1e-10)
			out<<x<<"\t"<<y<<endl;
		
	
	}
	
	
	
			
	return 0;

}

void int_histogram (vector <int> &c, ostream & out) {

	
	
	map<int, double> hist;
	
	double freq=1/double(c.size());
	
	for (int i=0; i<int(c.size()); i++) {
		
		map<int, double>::iterator itf=hist.find(c[i]);
		if (itf==hist.end())
			hist.insert(make_pair(c[i], 1.));
		else
			itf->second++;
	
	
	}
	
	
	for (map<int, double>::iterator it=hist.begin(); it!=hist.end(); it++)
		it->second=it->second*freq;
	
	prints(hist, out);



}

void int_histogram (deque <int> &c, ostream & out) {

	
	
	map<int, double> hist;
	
	double freq=1/double(c.size());
	
	for (int i=0; i<int(c.size()); i++) {
		
		map<int, double>::iterator itf=hist.find(c[i]);
		if (itf==hist.end())
			hist.insert(make_pair(c[i], 1.));
		else
			itf->second++;
	
	
	}
	
	
	for (map<int, double>::iterator it=hist.begin(); it!=hist.end(); it++)
		it->second=it->second*freq;
	
	prints(hist, out);



}

void int_histogram(int c, map<int, int> & hist) {
	
	
		
	map<int, int>::iterator itf=hist.find(c);
	if (itf==hist.end())
		hist.insert(make_pair(c, 1));
	else
		itf->second++;
	
	

}


void int_histogram(int c, map<int, double> & hist, double w) {
	
	
		
	map<int, double>::iterator itf=hist.find(c);
	if (itf==hist.end())
		hist.insert(make_pair(c, w));
	else
		itf->second+=w;
	
	

}


int print_cumulative (deque<double> & kws, string file, int number_of_step) {

	char buffer [file.size()+1];
	cast_string_to_char(file, buffer);
	
	ofstream expout (buffer);
	sort(kws.begin(), kws.end());
	
	int step=(kws.size()-1)/number_of_step;
	step=max(step, 1);
			
	for (int i=0; i<int(kws.size()); i++) if(i%step==0)
		expout<<kws[i]<<" "<<double(i+1)/(kws.size())<<endl;
	
	return 0;
}

int print_cumulative (deque<int> & kws, string file, int number_of_step) {
		
	
	
	char buffer [file.size()+1];
	cast_string_to_char(file, buffer);
	
	ofstream expout (buffer);
	sort(kws.begin(), kws.end());
	
	int step=(kws.size()-1)/number_of_step;
	step=max(step, 1);
			
	for (int i=0; i<int(kws.size()); i++) if(i%step==0)
		expout<<kws[i]<<" "<<double(i+1)/(kws.size())<<endl;			
			
	
			
		

	return 0;


}


int print_cumulative (vector<double> & kws, string file, int number_of_step) {
		
	
	
	char buffer [file.size()+1];
	cast_string_to_char(file, buffer);
	
	ofstream expout (buffer);
	sort(kws.begin(), kws.end());
	
	int step=(kws.size()-1)/number_of_step;
	step=max(step, 1);
			
	for (int i=0; i<int(kws.size()); i++) if(i%step==0)
		expout<<kws[i]<<" "<<double(i+1)/(kws.size())<<endl;
			
			
	
			
		

	return 0;


}


int print_cumulative (vector<int> & kws, string file, int number_of_step) {
		
	
	
	char buffer [file.size()+1];
	cast_string_to_char(file, buffer);
	
	ofstream expout (buffer);
	sort(kws.begin(), kws.end());
	
	int step=(kws.size()-1)/number_of_step;
	step=max(step, 1);
			
	for (int i=0; i<int(kws.size()); i++) if(i%step==0)
		expout<<kws[i]<<" "<<double(i+1)/(kws.size())<<endl;
			
			
	
			
		

	return 0;


}





void int_histogram(string infile, string outfile) {
	
	// this makes a int_histogram of integers from a file
	
	
	char b[infile.size()+outfile.size()+1];
	cast_string_to_char(infile, b);

	ifstream ing(b);
	deque<int> H;
	int h;
	while(ing>>h)
		H.push_back(h);
	
	
	cast_string_to_char(outfile, b);
	ofstream outg(b);
	int_histogram(H, outg);
	


}





void int_histogram(int c, map<int, int> & hist, int w) {
	
	
	
	map<int, int>::iterator itf=hist.find(c);
	if (itf==hist.end())
		hist.insert(make_pair(c, w));
	else
		itf->second+=w;
	
	
	
}


void int_histogram(const int & c, map<int, pair<int, double> >  & hist, const int & w1, const double  & w2) {
	
	
	
	map<int, pair<int, double> > ::iterator itf=hist.find(c);
	if (itf==hist.end())
		hist.insert(make_pair(c, make_pair(w1, w2) ));
	else {
		
		itf->second.first+=w1;
		itf->second.second+=w2;
		
	}
	
	
}



void int_histogram(int c, map<int, pair<double, double> > & hist, double w1, double w2) {
	
	
		
	map<int, pair<double, double> >::iterator itf=hist.find(c);
	if (itf==hist.end())
		hist.insert(make_pair(c, make_pair(w1, w2) ));
	else {
		
		itf->second.first+=w1;
		itf->second.second+=w2;
	
	}
	

}





void int_histogram(int c, map<int, pair<int, int> > & hist, int w1, int w2) {
	
	
	
	map<int, pair<int, int> >::iterator itf=hist.find(c);
	if (itf==hist.end())
		hist.insert(make_pair(c, make_pair(w1, w2) ));
	else {
		
		itf->second.first+=w1;
		itf->second.second+=w2;
		
	}
	
	
}


void int_histogram(int c, double w, map<int, DD> & hist) {
	
	if(hist.find(c)==hist.end()) {
		DD f;
		hist.insert(make_pair(c, f));
	}
	hist[c].push_back(w);

}




#endif


