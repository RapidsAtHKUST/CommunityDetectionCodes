


#include "./standard_package/standard_include.cpp"
#define bseparator 1.3


typedef multimap<double, int>::iterator map_idi;
typedef pair<map_idi, map_idi> pmap_idi;








class xypos {


	public:
				
		xypos(){};
		~xypos(){};
		
		bool erase(int);
		void edinsert(int , double , double );
		int size() {return xlabel.size();};
		void print_points(ostream &);
		void clear();
		int neighbors(int a, deque<int> & group_intsec);
		void set_all(map<int, double> & lab_x, map<int, double> & lab_y, map<int, double> & lab_r, double);
		int move(int a);
		int move_all();
		void get_new_pos(map<int, double> & lab_x, map<int, double> & lab_y);

	
	private:
		
		
		
		int xy_close(multimap<double, int> & m, double x, double dx, set<int> & close);



		map<int, double> label_radius;								
		multimap <double, int> xlabel;
		multimap <double, int> ylabel;
		map<int, pmap_idi >  label_points;
		double center_x;
		double center_y;
		
		

};


void xypos::set_all(map<int, double> & lab_x, map<int, double> & lab_y, map<int, double> & lab_r, double L) {
	
	// L is how far apart you want the module (1.2 should be ok)
	
	
	for(map<int, double> :: iterator itm=lab_x.begin(); itm!=lab_x.end(); itm++)
		edinsert(itm->first, itm->second, lab_y[itm->first]);
	
	
	for(map<int, double> :: iterator itm=lab_r.begin(); itm!=lab_r.end(); itm++)
		label_radius[itm->first]=L*itm->second;


	center_x=0;
	center_y=0;
	
	for(map<int, pmap_idi>:: iterator itm= label_points.begin(); itm!=label_points.end(); itm++) {
		
		center_x+=(itm->second.first)->first;
		center_y+=(itm->second.second)->first;

	
	}
	
	center_x/=label_points.size();
	center_y/=label_points.size();
	
	

}





void xypos::clear() {

	xlabel.clear();
	ylabel.clear();
	label_points.clear();


}


bool xypos::erase(int a) {		// this function erases element a if exists (and returns true)
	
	
	
	map<int, pmap_idi >::iterator itm= label_points.find(a);
	if(itm!=label_points.end()) {
		
		xlabel.erase(itm->second.first);
		ylabel.erase(itm->second.second);
		label_points.erase(itm);
		return true;
		
	}
	
	
	return false;
	
	
}



void xypos::edinsert(int a, double x, double y) {		// this function inserts element a (or edit it if it was already inserted)
	
	erase(a);
	
	map_idi itfx= xlabel.insert(make_pair(x, a));
	map_idi itfy= ylabel.insert(make_pair(y, a));
	
	
	label_points.insert(make_pair(a, make_pair(itfx, itfy)));
	
}



void xypos::print_points(ostream & outb) {
	
	outb<<"#x, y, label"<<endl;
	for(map<int, pmap_idi>:: iterator itm= label_points.begin(); itm!=label_points.end(); itm++)
		outb<<(itm->second.first)->first<<"\t"<<(itm->second.second)->first<<"\t"<<itm->first<<endl;
	
	
}


void xypos::get_new_pos(map<int, double> & lab_x, map<int, double> & lab_y) {

	
	lab_x.clear();
	lab_y.clear();
	
	for(map<int, pmap_idi>:: iterator itm= label_points.begin(); itm!=label_points.end(); itm++) {
		lab_x[itm->first]=(itm->second.first)->first;
		lab_y[itm->first]=(itm->second.second)->first;
		
	}
	


}


int xypos::neighbors(int a, deque<int> & group_intsec) {
	
	
	map<int, pmap_idi>:: iterator itm=label_points.find(a);
	double x=(itm->second.first)->first;
	double y=(itm->second.second)->first;
	double delta=2*label_radius[a];
	
	// returns all the guys which are delta close to (x,y)
	
	//cout<<"neighs of "<<a<<endl;

	
	set<int> xclose;
	set<int> yclose;
	
	xy_close(xlabel, x, delta, xclose);
	//prints(xclose);
	//cout<<">>>>>>>>>>><<<<<<<<<<<<<"<<endl;
	
	xy_close(ylabel, y, delta, yclose);
	//prints(yclose);
	//cout<<">>>>>>>>>>><<<<<<<<<<<<<"<<endl;

	
	xclose.erase(a);
	yclose.erase(a);
	
	
		
	group_intsec.clear();
	set_intersection(xclose.begin(), xclose.end(), yclose.begin(), yclose.end(), back_inserter(group_intsec));
	
	return 0;



}




int xypos::xy_close(multimap<double, int> & m, double x, double dx, set<int> & close) {
	
	
	
	//prints(m);
	
	
	//cout<<"looking for "<<x - 1e-2<<endl;
	map_idi xit=m.lower_bound(x - 1e-10);
	map_idi xit_m=xit;
	
	while(true) {
		
		
		
		
		
		if(xit==m.end())
			break;
		
		//cout<<"xit: "<<xit->first<<" "<<xit->second<<endl;
		
		if(xit->first - x <= dx) {
			close.insert(xit->second);
			//cout<<"-> "<<xit->first - x<<endl;
		}
		else
			break;
		
		xit++;
	}
	
	
	while(true) {
		
		if(xit_m==m.begin())
			break;

		
		xit_m--;
		
		//cout<<"xitm: "<<xit_m->first<<" "<<xit_m->second<<endl;

		
		
		if( x - xit_m->first<= dx) {
			close.insert(xit_m->second);
			//cout<<"-> "<<x - xit_m->first<<endl;
		}
		else
			break;
		
		
		
		
	}
	
	
	/*cout<<"close:"<<endl;
	prints(close);*/
	
	
	
}



int xypos::move(int a) {
	
	
	
	map<int, pmap_idi>:: iterator itm=label_points.find(a);
	double x=(itm->second.first)->first;
	double y=(itm->second.second)->first;
	
	/*cout<<"moving::: "<<a<<endl;
	cout<<"x... "<<x<<" "<<y<<endl;*/
	
	deque<int> neighs;
	neighbors(a, neighs);
	
		
	deque<int> eff;
	
	
	
	//cout<<"neighbors ************** "<<endl;
	for(int i=0; i<neighs.size(); i++) {
		
		
		map<int, pmap_idi>:: iterator itm2=label_points.find(neighs[i]);
		double x2=(itm2->second.first)->first;
		double y2=(itm2->second.second)->first;
		
		//cout<<neighs[i]<<" "<<x2<<" "<<y2<<endl;
		
		double l= (x-x2)*(x-x2) + (y-y2)*(y-y2);
		
		//cout<<"l: "<<sqrt(l)<<" "<<label_radius[a] + label_radius[neighs[i]]<<endl;
		if(sqrt(l)<label_radius[a] + label_radius[neighs[i]])
			eff.push_back(neighs[i]);
		
	
	}
	
	//cout<<"*****************"<<endl;
	
	if(eff.size()==0)
		return -1;
	
	
	for(int i=0; i<eff.size(); i++) {
	
		int rn=eff[i];
		
		map<int, pmap_idi>:: iterator itm2=label_points.find(rn);
		double x2=(itm2->second.first)->first;
		double y2=(itm2->second.second)->first;
		
		double di=sqrt((x-x2)*(x-x2) + (y-y2)*(y-y2));
		double bi=bseparator*(label_radius[a] + label_radius[rn] - di);
		
		
		double o1=(x-center_x)*(x-center_x) + (y-center_y)*(y-center_y);
		double o2=(x2-center_x)*(x2-center_x) + (y2-center_y)*(y2-center_y);
		
		
		
		//cout<<di<<"<----"<<endl;
		
		
		if(o1>o2) {
			
			
			double cos_, sin_;
			if(di>0) {
				cos_=(x-x2) / di;
				sin_=(y-y2) / di;
			} else {
				
				cos_=1.;
				sin_=0;
			}
				
			double nx, ny;
			nx = x + bi * cos_;
			ny= y + bi * sin_;
			edinsert(a, nx, ny);
		}
		
		
		else {
			
			
			double cos_, sin_;
			if(di>0) {
				cos_=(x2-x) / di;
				sin_=(y2-y) / di;
			} else {
				
				cos_=1.;
				sin_=0;
			}
			
			
			
			
			double nx, ny;
			nx = x2 + bi * cos_;
			ny= y2 + bi * sin_;
			edinsert(rn, nx, ny);
		
		
		}
	}
	//cout<<center_x<<" "<<center_y<<" "<<o1<<" "<<rn<<endl;
 	
	
	return 0;
	



}



int xypos::move_all() {
	
	
	deque<int> points;
	for(map<int, pmap_idi>:: iterator itm= label_points.begin(); itm!=label_points.end(); itm++)
		points.push_back(itm->first);
	
	
	int stopper=0;
	
	while(stopper<100) {
		
		stopper++;
		cout<<"checking overlaps... "<<stopper<<endl;
	
		
		bool again=false;
		for(int i=0; i<points.size(); i++) {
		
			int h=move(points[i]);
			if(h==0)
				again=true;
		
	
		}
		
		if(again==false)
			break;
	
	
		
	}
	
	
	return 0;
	
	
}





