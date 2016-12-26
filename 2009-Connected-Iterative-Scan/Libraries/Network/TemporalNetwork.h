#ifndef RPI_TEMPORAL_NETWORK
#define RPI_TEMPORAL_NETWORK

#include <set>
#include <map>
#include <iostream>
#include <vector>
#include <memory>

#include "../Files/IOX.h"
#include "../Files/StringEx.h"
#include "Network.h"

using namespace std;

typedef set < shared_ptr < string >, cmp_str_ptr > community;

/**
 *@class TemporalNetwork
 *
 */
class TemporalNetwork{
 public:
  TemporalNetwork(){};
  ~TemporalNetwork(){};

  shared_ptr < Network >  AddNetwork( const string& filename, const string& delimiters, const bool& directed ); //Add new network to series
  void AddCommunities( const string& filename, const string& delimiters ); //Add new community structure
  set < string >::const_iterator getFirstVertex( ) { return vertex_set.begin(); }
  set < string >::const_iterator getLastVertex( ) { return vertex_set.end(); }

  int CommSteps() { return communities.size(); }
  int NetSteps() { return networks.size(); }
  
  set < community, cmp_set_str >& ComStructure ( int timestep ) { 
    if ( ( timestep < 0 ) || ( (unsigned int)timestep >= communities.size() ) ) return communities[0];
    return communities[timestep]; 
  }
  
 private:
  set < string > vertex_set; //List of vertices in any of the networks

  vector < shared_ptr < Network > > networks; //List of networks (in order from earliest to latest)
  vector < set < community, cmp_set_str > > communities;
};

#endif
