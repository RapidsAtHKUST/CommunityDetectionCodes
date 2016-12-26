#ifndef RPI_NETWORK
#define RPI_NETWORK

#include <map>
#include <memory>

using namespace std;

struct cmp_str_ptr{
  bool operator() (const shared_ptr < string >& lhs, const shared_ptr < string >& rhs) const{
    return ((*lhs).compare(*rhs) < 0);
  }
};

struct cmp_set_str{
  bool operator () (const set < shared_ptr < string >, cmp_str_ptr >& lhs, const set < shared_ptr < string >, cmp_str_ptr >& rhs){
    if ( lhs.size() != rhs.size() ) return lhs.size() < rhs.size();

    cmp_str_ptr cmp;

    set < shared_ptr < string >, cmp_str_ptr >::const_iterator it_s, it_t;
    it_t = rhs.begin(); 
    for ( it_s = lhs.begin(); it_s != lhs.end(); it_s++, it_t++ ){
      if ( cmp (*it_s, *it_t) != cmp(*it_t, *it_s) ) return cmp ( *it_s, *it_t);
    }

    return false;
  }
};

class Network {
 public:
  Network(){};
  ~Network(){};

  void addEdge( shared_ptr < string > fr, shared_ptr < string > to, const double& weight, const bool& directed);
  void removeEdge ( shared_ptr < string > fr, shared_ptr < string > to, const bool& directed );
  map < shared_ptr < string >, double, cmp_str_ptr > GetNeighborhood ( shared_ptr < string > node );  

  bool hasEdge ( string fr, string to );
  int Degree ( shared_ptr < string > node );

  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator Edgelist_begin(){ return A.begin(); };
  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator Edgelist_end(){ return A.end(); };

  void Print(string filename, string delimiter);
  
 private:
  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr > A; //Adjacency matrix (as edge list)
};

#endif
