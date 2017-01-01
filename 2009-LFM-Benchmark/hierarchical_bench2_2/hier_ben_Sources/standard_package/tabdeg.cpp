
#include <math.h>
#include <iostream>
#include <deque>
#include <set>
#include <vector>
#include <map>
#include <string> 
#include <fstream>
#include <ctime>
#include <iterator>
#include <algorithm>



using namespace std;


typedef multiset<pair<double, int> >  muspi;



class tabdeg {


	public:
				
		tabdeg(){};
		~tabdeg(){};
		
		bool is_internal(int );
		void edinsert(int , double );
		bool erase(int);
		double indegof(int );
		int size() {return nodes_indeg.size();};
		void print_nodes(ostream &);
		int best_node();
		int worst_node();	



	private:

		map <int, muspi::iterator> nodes_indeg;				// for each node belonging to the cluster, here you have where to find the internal degree
		muspi number_label;
	
	
	

};


bool tabdeg::is_internal(int a) {

	
	map<int, muspi::iterator >::iterator itm= nodes_indeg.find(a);
	if(itm==nodes_indeg.end())
		return false;
	else
		return true;




}

void tabdeg::edinsert(int a, double kp) {		// this function inserts element a (or edit it if it was already inserted)
	
	erase(a);
		
	muspi::iterator itms= number_label.insert(make_pair(kp, a));
	nodes_indeg.insert(make_pair(a, itms));
	
	


}


bool tabdeg::erase(int a) {		// this function erases element a if exists (and returns true)
	
	
	
	map<int, muspi::iterator >::iterator itm= nodes_indeg.find(a);
	if(itm!=nodes_indeg.end()) {
		
		number_label.erase(itm->second);
		nodes_indeg.erase(itm);
		return true;
		
	}
		
	
	return false;
	

}


double tabdeg::indegof(int a) {		// return the internal degree of a, 0 if it's not internal
	
	map<int, muspi::iterator >::iterator itm= nodes_indeg.find(a);
	if(itm!=nodes_indeg.end())
		return itm->second->first;
	else
		return 0;
		


}





void tabdeg::print_nodes(ostream & outb) {
	
	for(map<int, muspi::iterator >::iterator itm= nodes_indeg.begin(); itm!=nodes_indeg.end(); itm++)
		outb<<itm->first<<"\t"<<itm->second->first<<endl;
	
	
	
}

int tabdeg::best_node() {

	
	muspi::iterator itm= number_label.end();
	if(number_label.size()==0)
		return -1;
	
	
	itm--;
	return itm->second;
	


}

int tabdeg::worst_node() {

	
	muspi::iterator itm= number_label.begin();
	if(number_label.size()==0)
		return -1;
	
	return itm->second;
	


}



int main() {

	
	
	tabdeg C;
	C.edinsert(2, 14);
	C.edinsert(7, 24);
	C.edinsert(10, -1.2);
	C.edinsert(11, 12.8);
	
	cout<<"--------------------"<<endl;
	C.print_nodes(cout);
	
	C.erase(11);
	
	cout<<"--------------------"<<endl;
	C.print_nodes(cout);
	
	
	cout<<"indegof "<<C.indegof(10)<<endl;
	
	cout<<"best node "<<C.worst_node()<<endl;
	int best_node=C.worst_node();
	C.erase(best_node);
	cout<<"--------------------"<<endl;
	C.print_nodes(cout);


	
	
	
	
	
	
	return 0;



}







