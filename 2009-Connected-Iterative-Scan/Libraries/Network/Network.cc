#include "Network.h"

/**
 *@fn void AddEdge( shared_ptr < string > fr, shared_ptr < string > to, const double& weight, const bool& directed )
 *
 * Add an edge into the network. If there is already an edge there, the weight is replaced. 
 *
 *@param fr One endpoint of edge. In directed graph, is the source
 *@param to Other endpoint of edge. In directed graph, is the target
 *@param weight Weight to associate with edge
 *@param directed Indicator as to whether or not the edge is directed
 */

void Network::addEdge( shared_ptr < string > fr, shared_ptr < string > to, const double& weight, const bool& directed ){
  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator it_n;
  map < shared_ptr < string >, double, cmp_str_ptr >::iterator it_e;

  //Add the fr->to edge
  if ( (it_n = A.find( fr ) ) == A.end() ){
    map < shared_ptr < string >, double, cmp_str_ptr > hood;
    hood.insert(pair < shared_ptr < string >, double > (to, weight));
    A.insert(pair < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr > > (fr, hood));
  } else {
    if ( (it_e = it_n->second.find(to) ) == it_n->second.end() ){
      it_n->second.insert(pair < shared_ptr < string >, double > (to, weight));
    } else {
      it_e->second = weight;
    }
  }

  //If the network is undirected, add to->fr as well
  if ( !directed ){
    if ( (it_n = A.find( to ) ) == A.end() ){
      map < shared_ptr < string >, double, cmp_str_ptr > hood;
      hood.insert(pair < shared_ptr < string >, double > (fr, weight));
      A.insert(pair < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr > > (to, hood));
    } else {
      if ( (it_e = it_n->second.find(fr) ) == it_n->second.end() ){
	it_n->second.insert(pair < shared_ptr < string >, double > (fr, weight));
      } else {
	it_e->second = weight;
      }
    }
  }

}


/**
 *@fn map < shared_ptr < string >, double, cmp_str_ptr > Network::getNeighborhood ( shared_ptr < string > node )
 *
 *@param node Vertex to retrieve neighborhood information about
 */
map < shared_ptr < string >, double, cmp_str_ptr > Network::GetNeighborhood ( shared_ptr < string > node ){
  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator it_n;
  
  if ( (it_n = A.find(node)) == A.end() ) {
    map < shared_ptr < string >, double, cmp_str_ptr > empty;
    return empty;
  } else {
    return it_n->second;
  }
}

bool Network::hasEdge( string fr, string to ){
  shared_ptr < string > base ( new string ( fr ) ), target ( new string ( to ) );

  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator it_n;
  map < shared_ptr < string >, double, cmp_str_ptr >::iterator it_e;
  
  if ( (it_n = A.find(base)) != A.end () ){
    if ( it_n->second.find(target) != it_n->second.end() ){
      return true;
    } else {
      return false;
    }
  } else {
    if ( (it_n = A.find(target)) != A.end() ){
      if ( it_n->second.find(base) != it_n->second.end() ){
	return true;
      } else {
	return false;
      }
    } else {
      return false;
    }
  }
}

void Network::Print ( string filename, string delimiter ){
  ofstream fout ( filename.c_str() );

  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator it_n;
  map < shared_ptr < string >, double, cmp_str_ptr >::iterator it_e;
  
  for ( it_n = A.begin(); it_n != A.end(); it_n++ ){
    for ( it_e = it_n->second.begin(); it_e != it_n->second.end(); it_e++ ){
      fout << *(it_n->first) << delimiter << *(it_e->first) << delimiter << it_e->second << endl;
    }
  }
  
  fout.close();
}


void Network::removeEdge ( shared_ptr < string > fr, shared_ptr < string > to, const bool& directed ){
  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator it_n;
  map < shared_ptr < string >, double, cmp_str_ptr >::iterator it_e;

  if ( ( it_n = A.find ( fr ) ) != A.end() ){
    if ( ( it_e = it_n->second.find( to ) ) != it_n->second.end() ){
      it_n->second.erase( it_e );
    }
  }

  if ( !directed ){
    if ( ( it_n = A.find ( to ) ) != A.end() ){
      if ( ( it_e = it_n->second.find( fr ) ) != it_n->second.end() ){
	it_n->second.erase( it_e );
      }
    }
  }
}

int Network::Degree ( shared_ptr < string > node ){
  map < shared_ptr < string >, map < shared_ptr < string >, double, cmp_str_ptr >, cmp_str_ptr >::iterator it_n;
  
  if ( ( it_n = A.find ( node ) ) == A.end() ) return 0;
  else return it_n->second.size();
}
