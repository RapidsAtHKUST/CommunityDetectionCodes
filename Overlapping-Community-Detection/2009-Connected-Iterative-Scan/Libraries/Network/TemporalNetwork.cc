#include "TemporalNetwork.h"

/**
 *@fn TemporalNetwork::AddNetwork ( const string& filename, const string& delimiters )
 *
 *Adds a network structure to the end of the temporal network represented by this structure.
 *
 *@TODO - Add parameter to notify if the network is directed or undirected.
 *      - Add option to add a network in any time point of the temporal network
 *
 *@param filename Name of the file to load the information from. Each line in the file
 *                     should be a single edge - node1|node2|edge_weight. If the network
 *                     is undirected, it doesn't matter if both directions are in the 
 *                     file. If directed, the edge format is fr|to|edge_weight.
 *@param delimiters Characters to be used as the delimiter for the network file
 *
 */
shared_ptr < Network > TemporalNetwork::AddNetwork ( const string& filename, const string& delimiters, const bool& directed ){
  ifstream fin;                                              //Open file
  openFileHarsh(&fin, filename);

  vector < string > fields;                                  //Set up parameters
  shared_ptr < Network > result ( new Network() );

  while ( fline_tr( &fin, &fields, delimiters ) ){
    if (fields.size() != 3) continue;                        //Simple format check
    
    for (int i = 0; i < 2; i++){                             //Add new edges to list
      if ( vertex_set.find(fields[i]) == vertex_set.end() ){
	vertex_set.insert(fields[i]);
      }
    }

    pair < double, bool > ret = check_str_to<double>(fields[2]);  //Get edge weight
    if ( !ret.second ) continue;                          //Wrong format

    result->addEdge(shared_ptr<string>( new string ( *(vertex_set.find(fields[0])) ) ), 
	      shared_ptr<string>( new string ( *(vertex_set.find(fields[1])) ) ), 
	      ret.first, directed );                      //Add to network
  }

  networks.push_back(result);                                     //Add to time series

  fin.close();                                               //Clean up
  
  return  ( networks[networks.size() - 1] );
}

void TemporalNetwork::AddCommunities ( const string& filename, const string& delimiters ){
  ifstream fin;                                              //Open file
  openFileHarsh(&fin, filename);

  vector < string > fields;                                  //Set up parameters
  set < community, cmp_set_str > structure;

  while ( fline_tr( &fin, &fields, delimiters ) ){  
    community next;
    for (int i = 0; i < fields.size(); i++){                             //Add new edges to list
      if ( vertex_set.find(fields[i]) == vertex_set.end() ){
	vertex_set.insert(fields[i]);
      }
      
      next.insert( shared_ptr < string > ( new string ( *(vertex_set.find(fields[i]) ) ) ) );
    }

    structure.insert(next);
  }

  communities.push_back( structure );

  fin.close();                      //Clean up
}
