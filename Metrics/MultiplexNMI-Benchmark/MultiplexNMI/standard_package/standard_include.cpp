#if !defined(STANDARD_INCLUDE_INCLUDED)
#define STANDARD_INCLUDE_INCLUDED	


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
#include <numeric>
#include <queue>
#include <stack>
#include <list>
#include <limits>
#include <sstream>


using namespace std;


typedef unsigned int UI;
typedef deque<double> DD;
typedef deque<int> DI;
typedef set<int> SI;
typedef deque<deque<int> > int_matrix;
typedef map<int, double> mapid;
typedef map<int, int> mapii;
typedef map<string, int> mapsi;
typedef map<int, string> mapis;
typedef map<string, double> mapsd;

#define RANGE_loop(i, s) for(UI i= 0; i<s.size(); i++)
#define IT_loop(template_, itm, s) for(template_::iterator itm= s.begin(); itm!=s.end(); itm++)
#define IT_const_loop(template_, itm, s) for(template_::const_iterator itm= s.begin(); itm!=s.end(); itm++)
#define UP_TO_loop(i, s) for(UI i= 0; i<s; i++)


// Ex. for using IT_loop
// IT_loop(mapii, itm, hist)



#include "cast.cpp"
#include "assert.cpp"
#include "deque_numeric.cpp"
#include "print.cpp"
#include "random.cpp"
#include "partition.cpp"
#include "map_utilities.cpp"
#include "combinatorics.cpp"
#include "histograms.cpp"
#include "mutual.cpp"
//#include "pajek.cpp"


#endif
