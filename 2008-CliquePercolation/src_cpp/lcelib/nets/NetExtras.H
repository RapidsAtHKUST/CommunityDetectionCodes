/*
lcelib, A high performance library for complex networks research
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

 
// lcelib/nets/NetExtras.H
// Auxiliary functions for use with the SymmNet class.
// 
// Some of these functions, such as ClearNet or ConnectivityCheck,
// could be methods within the SymmNet class, but are now written
// using the interface. That's why the file is named NetExtras.H.
// 
// Authors: Riitta Toivonen, Jussi Kumpula



#ifndef NETEXTRAS_H
#define NETEXTRAS_H
#include "../Containers.H"
#include "../Nets.H"
#include "../Randgens.H"
#include "Dijkstrator.H"
#include "../misc/KruskalTree2.H"

#include <cassert>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <ctime>
#include <fstream>






/*

 Functions included 
 ------------------  

 readNet                (added Dec 6 2005, Riitta)
 readNet3                (added Aug 16 2006, Jussi)
 readNet4               (2006 Riitta ?)
 readNet_mutual         (June 7 2007, Lauri)
 outputEdges            (Nov 9 2005, Riitta)
 outputEdgesAndWeights  (Nov 9 2005, Riitta)
 outputEdgesAndWeights2  (Aug 16 2006, Jussi)
 copyNet                (Jussi) 
 numberOfEdges          (Jussi) 
 numberOfTriangles      (Jan 4 2006, Riitta) 
 ConnectivityCheck      (Riitta)
 ClearNet               (Riitta)
 outputOverlap          (April 4 2006, Riitta)
 shuffle                (added Sept 14 2006, Riitta) 
 overlap                (added Sept 14 2006, Riitta)  
 findLargestComponent   (added June 14 2007, Jussi)
 collapseIndices        (added June 14 2007, Lauri)
 clusteringCoefficient  (added May 22 2009, Lauri)
 clusteringCoefficient_Onnela  (added May 22 2009, Lauri)
 clusteringCoefficient_Barrat  (added May 22 2009, Lauri)
 clusteringCoefficient_Ahnert  (added May 22 2009, Lauri)

Feel free to add whatever useful bits you have that belong to this category.
A reasonable amount of testing is expected.

Suggestions, corrections etc. also welcomed. 

Usage examples for nontrivial functions should be found in 
lcelib/nets/Examples/

 */ 




/* function readNet */
/* Reads a net from stdin and returns a pointer to a new SymmNet
 * instance. The number of edges and nodes does not need to be 
 * known beforehand - the function deduces them from the input. 
 * This can be useful if you need to randomize a large ensemble of 
 * networks. 
 *  
 * For more info and an example on how to use this function, 
 * see lcelib/nets/Examples/readNetExample.cpp
 * 
 * EdgeDataType must be a type with an extractor (cin >> data).
 * 
 * If a copy-constructor is written for SymmNet, this function could
 * be written and called more neatly. Or, if a SymmNet was written that 
 * can handle changing numbers of nodes, and an extractor was written 
 * for it, we could throw away this function and just say 
 * 
 *    NetType net; 
 *    std::cin >> net;
 * 
 * (which means Jorkki would have done all the dirty work). 
 * In the meantime, use this laborious construction. 
 */



template<typename EdgeDataType>
SymmNet<EdgeDataType>* readNet()
{
  typedef SymmNet<EdgeDataType> MyNetType;

  // Read edge data into a temporary buffer.
  /*  struct Edge {
      size_t source;
      size_t dest;
      EdgeDataType data; 
      };
      std::vector<Edge> edges;
      Turned out to be impossible to make a vector of Edges.
      Making separate vectors for source, dest and data.
  */
  
  std::vector<size_t> edgeSource;
  std::vector<size_t> edgeDest;
  std::vector<EdgeDataType> edgeData;
  size_t nodeCount = 0;

  while (std::cin) {
    size_t source, dest;
    EdgeDataType data;
    std::string line;
    std::getline(std::cin, line);  // Read a line from input into a string.
    if (!line.empty()) {
      std::istringstream is(line);  // Extract data using a stringstream.
      is >> source;
      is >> dest;
      is >> data;
      if (!is) {
	std::cerr << "\nError in reading input.\n"
		  << "Possibly a line containing too few values, or a header line.\n\n"; 
	exit(1); 
      }

      // Track the maximum node index.
      if (source >= nodeCount)
	nodeCount = source + 1;
      if (dest >= nodeCount)
	nodeCount = dest + 1;
      
      edgeSource.push_back(source);
      edgeDest.push_back(dest);
      edgeData.push_back(data);
    }
  }

  // Construct the net.
  std::auto_ptr<MyNetType> netPointer(new MyNetType(nodeCount));
  MyNetType& net = *netPointer;  // Create a reference for easier access.
  
  // Add edges to the net.
  for (size_t i = 0; i < edgeSource.size(); ++i) {
    size_t source = edgeSource.at(i);
    size_t dest = edgeDest.at(i);
    EdgeDataType data = edgeData.at(i);
    
    if (source == dest) {     
      std::cerr << "\nInput file contains a loop edge.\n\n";
      exit(1);
    }
    if (net(source).contains(dest)) {
      std::cerr << "\nInput file contains same edge twice (edge "<< source <<"-"<< dest<<"). ";
      std::cerr << "Using the latter weight data.";
    }
    net[source][dest] = data;
    assert(net[source][dest] == data);
  }
  
  std::cerr << "\n\nreadNet: read in " << nodeCount << " nodes and ";
  std::cerr << edgeSource.size() << " links.\n";
  std::cerr << "Reading data complete.\n\n";
  
  return netPointer.release(); // release the pointer so that it is not destroyed
			       // (we want to return it)
}
// <--- readNet
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 



// readNet2 ------------------------------->
// This is almost a direct copy of the readNet-function.
// However, this function can handle all kinds of networks, for example  
// SymmNet<EdgeData, ValueTable, ExplSumTreeTable>.
// Furthermore, this function can read files which do not specify
// the weights (set parameter weights=0). If weights are given set parameter
// weights=1.  Also, files which contain on their first line some headers
// etc. can be read by setting himmeli=1, so that the first line is ignored.
// 
// call example:
// std::auto_ptr<NetType> netPointer(readNet2<NetType>(1,0));
// NetType& net = *netPointer;  // Create a reference for easier handling of net.

template<typename NetType>
NetType * readNet2(const size_t weights, const size_t himmeli){

  typedef typename NetType::EdgeData EdgeDataType;
  
  std::vector<size_t> edgeSource;
  std::vector<size_t> edgeDest;
  std::vector<EdgeDataType> edgeData;
  size_t nodeCount = 0;

  if (himmeli) {                   // first line in himmeli edge data file has to be skipped
    std::string line;
    std::getline(std::cin, line);  // Read a line from input and discard it...
  }

  while (std::cin) {

    size_t source, dest;
    EdgeDataType data;
    std::string line;
    std::getline(std::cin, line);   // Read a line from input into a string.
    if (!line.empty()) {
      std::istringstream is(line);  // Extract data using a stringstream.
      is >> source;
      is >> dest;
      if (weights) is >> data;      // if weights are given use them
      else data=1;                  // else set all weights to 1
      if (!is) {
	std::cerr << "\nError in reading input.\n"
		  << "Possibly a line containing too few values, or a header line.\n"
		  << "source = " << source << ", dest = " << dest << " " <<  data << "\n\n";
	exit(1); 
      }

      // Track the maximum node index.
      if (source >= nodeCount)
	nodeCount = source + 1;
      if (dest >= nodeCount)
	nodeCount = dest + 1;

      edgeSource.push_back(source);
      edgeDest.push_back(dest);
      edgeData.push_back(data);
    }
  }

  // Construct the net.
  std::auto_ptr<NetType> netPointer(new NetType(nodeCount));
  NetType& net = *netPointer;  // Create a reference for easier access.
  
  // Add edges to the net.
  for (size_t i = 0; i < edgeSource.size(); ++i) {
    size_t source = edgeSource.at(i);
    size_t dest = edgeDest.at(i);
    EdgeDataType data = edgeData.at(i);
    
    if (source != dest && !net(source).contains(dest)) {
      net[source][dest] = data;
      assert(net[source][dest] == data);
    }
  }
  
  return netPointer.release(); // release the pointer so that it is not destroyed
			       // (we want to return it)
}
// <---------------- readNet2 --------------------------




/*
// readNet_mutual -------------------------------> 
//
// This is almost a direct copy of the readNet2-function.  However,
// this function reads a directed network and saves it as an
// undirected network where a link between i and j exist only if both
// links (i,j) and (j,i) exist in the original network. The weigh of
// the new undirected link is the average of the weights of the
// directed links.
// 
// call example:
// std::auto_ptr<NetType> netPointer(readNet_mutual<NetType>(1,0));
// NetType& net = *netPointer;  // Create a reference for easier handling of net.

template<typename NetType>
NetType * readNet_mutual(const size_t weights, const size_t himmeli)
{

  typedef typename NetType::EdgeData EdgeDataType;
  typedef std::vector<EdgeDataType> WeightVector;

  std::map<size_t, std::vector<size_t> > edgeMap;

  std::vector<size_t> edgeSource;
  std::vector<size_t> edgeDest;
  std::vector<EdgeDataType> edgeData;
  size_t nodeCount = 0;

  if (himmeli) // first line in himmeli edge data file has to be skipped
    {                   
      std::string line;
      std::getline(std::cin, line);  // Read a line from input and discard it...
    }

  while (std::cin)
    {

      size_t source, dest;
      EdgeDataType data;
      std::string line;
      std::getline(std::cin, line);   // Read a line from input into a string.
      
      if (!line.empty())
	{
	  std::istringstream is(line);  // Extract data using a stringstream.
	  is >> source;
	  is >> dest;
	  if (weights) is >> data;      // if weights are given use them
	  else data=1;                  // else set all weights to 1
	  if (!is) {
	    std::cerr << "\nError in reading input.\n"
		      << "Possibly a line containing too few values, or a header line.\n\n"; 
	    exit(1); 
	  }
	  
	  // Track the maximum node index.
	  if (source >= nodeCount)
	    nodeCount = source + 1;
	  if (dest >= nodeCount)
	    nodeCount = dest + 1;
	  
	  // Check if a link has been already read in reverse direction.
	  bool edge_found = false;
	  size_t vector_index = 0;
	  std::map<size_t, std::vector<size_t> >::iterator edgemapiter = edgeMap.find(dest);
	  if ( edgemapiter != edgeMap.end() )
	    {
	      for (std::vector<size_t>::iterator veciter = (edgemapiter->second).begin();
		   veciter != (edgemapiter->second).end();
		   veciter++)
		{
		  if (*veciter == source)
		    {
		      edge_found = true;
		      break;
		    }
		  vector_index++;
		}
	    }
	  
	  // Add edge to network if it was found to exist in both directions.
	  // Otherwise add edge to map.
	  if (edge_found)
	    {
	      edgeSource.push_back(dest);
	      edgeDest.push_back(source);
	      
	      // Calculate average weight
	      std::map<size_t, std::vector<EdgeDataType> >::iterator weightmapiter = weightMap.find(dest);
	      assert(weightmapiter != weightMap.end());
	      edgeData.push_back(0.5*(data + (weightmapiter->second).at(vector_index)));
	    }
	  else
	    {
	      // Add new node to map. If source was not found in edgeMap,
	      // it must be added to the map. Otherwise we may just extend
	      // the old vectors in the map with a new destination.
	      if ( edgemapiter == edgeMap.end() )
		{	  
		  std::vector<size_t> edge_vec;
		  std::vector<EdgeDataType> weight_vec;
		  edge_vec.push_back(dest);
		  weight_vec.push_back(data);
		  edgeMap[source] = edge_vec;
		  weightMap[source] = weight_vec;
		}
	      else
		{
		  (edgeMap[source]).push_back(dest);
		  (weightMap[source]).push_back(data);
		}
	      
	    }
	  
	} // End if (!line.empty())

    } // End while (std::cin)

  // Construct the net.
  std::auto_ptr<NetType> netPointer(new NetType(nodeCount));
  NetType& net = *netPointer;  // Create a reference for easier access.
  
  // Add edges to the net.
  for (size_t i = 0; i < edgeSource.size(); ++i)
    {
      size_t source = edgeSource.at(i);
      size_t dest = edgeDest.at(i);
      EdgeDataType data = edgeData.at(i);
    
      if (source != dest && !net(source).contains(dest))
	{
	  net[source][dest] = data;
	  assert(net[source][dest] == data);
	}
    }
  
  return netPointer.release(); // release the pointer so that it is not destroyed
			       // (we want to return it)
}
// <---------------- readNet_mutual --------------------------
*/







// readNet3 ------------------------------->
// This file reads the network from a specified file, not standard in
//  use the option 'himmeli' = 1 if Himmeli headers in the edge file, 0 otherwise
template<typename NetType>
NetType * readNet3(const size_t weights, const size_t himmeli, const char * fileName)
{
  typedef typename NetType::EdgeData EdgeDataType;
  
  std::vector<size_t> edgeSource;
  std::vector<size_t> edgeDest;
  std::vector<EdgeDataType> edgeData;
  size_t nodeCount = 0;
  
  std::string line;
  std::ifstream myfile(fileName);
  if (myfile.is_open()) {
    
    if (himmeli) {                   // first line in himmeli edge data file has to be skipped
      getline(myfile,line);
    }
    
    while (!myfile.eof()) {
      
      size_t source, dest;
      EdgeDataType data;
      getline(myfile, line);   // Read a line from input into a string.
      if (!line.empty()) {
	std::istringstream is(line);  // Extract data using a stringstream.
	is >> source;
	is >> dest;
	if (weights) is >> data;      // if weights are given use them
	else data=1;                  // else set all weights to 1
	if (!is) {
	  std::cerr << "\nError in reading input.\n"
		    << "Possibly a line containing too few values, or a header line.\n\n"; 
	  exit(1); 
	}
	
	// Track the maximum node index.
	if (source >= nodeCount)
	  nodeCount = source + 1;
	if (dest >= nodeCount)
	  nodeCount = dest + 1;
	
	edgeSource.push_back(source);
	edgeDest.push_back(dest);
	edgeData.push_back(data);
      }
    }
    
    myfile.close();
    
    // Construct the net.
    std::auto_ptr<NetType> netPointer(new NetType(nodeCount));
    NetType& net = *netPointer;  // Create a reference for easier access.
    
    std::cerr << nodeCount << "\n";
    
    // Add edges to the net.
    for (size_t i = 0; i < edgeSource.size(); ++i) {
      size_t source = edgeSource.at(i);
      size_t dest = edgeDest.at(i);
      EdgeDataType data = edgeData.at(i);
      
      if (source != dest && !net(source).contains(dest)) {
	net[source][dest] = data;
	assert(net[source][dest] == data);
      }
    }
    
    return netPointer.release(); // release the pointer so that it is not destroyed
    // (we want to return it)
  }

  return 0;
}

// <---------------- readNet3 --------------------------











// readNet4 ------------------------------->
// This file reads the network from a specified file, not standard in
// (wrote this since readNet3 wouldn't compile with my compiler - Aug 18 2006 Riitta)
template<typename NetType>
NetType * readNet4(const size_t weights, const size_t himmeli, const char * fileName)
{
  typedef typename NetType::EdgeData EdgeDataType;
  
  
  
  std::vector<size_t> edgeSource;
  std::vector<size_t> edgeDest;
  std::vector<EdgeDataType> edgeData;
  size_t nodeCount = 0;
  
  
  std::string line;
  std::ifstream myfile(fileName);
  //if (myfile.is_open()) {
  
  if (himmeli) {                   // first line in himmeli edge data file has to be skipped
    getline(myfile,line);
  }
  while (!myfile.eof()) {
    
    size_t source, dest;
    EdgeDataType data;
    getline(myfile, line);   // Read a line from input into a string.
    if (!line.empty()) {
      std::istringstream is(line);  // Extract data using a stringstream.
      is >> source;
      is >> dest;
      if (weights) is >> data;      // if weights are given use them
      else data=1;                  // else set all weights to 1
      if (!is) {
	std::cerr << "\nError in reading input.\n"
		  << "Possibly a line containing too few values, or a header line.\n\n"; 
	exit(1); 
      }
      
      // Track the maximum node index.
      if (source >= nodeCount)
	nodeCount = source + 1;
      if (dest >= nodeCount)
	  nodeCount = dest + 1;
      
      edgeSource.push_back(source);
      edgeDest.push_back(dest);
	edgeData.push_back(data);
    }
  }
  myfile.close();
  
    
  
  
  // Construct the net.
  std::auto_ptr<NetType> netPointer(new NetType(nodeCount));
  NetType& net = *netPointer;  // Create a reference for easier access.
  
  
  //  std::cerr << nodeCount << "\n";
  
  // Add edges to the net.
  for (size_t i = 0; i < edgeSource.size(); ++i) {
    size_t source = edgeSource.at(i);
    size_t dest = edgeDest.at(i);
    EdgeDataType data = edgeData.at(i);
    
    if (source == dest) {     
      std::cerr << "\nInput file contains a loop edge.\n\n";
      exit(1);
    }
    if (net(source).contains(dest)) {
      std::cerr << "\nInput file contains same edge twice (edge "<< source <<"-"<< dest<<"). ";
      std::cerr << "Using the latter weight data.";
    }
    net[source][dest] = data;
    assert(net[source][dest] == data);
  }
  
  std::cerr << "\n\nreadNet: read in " << nodeCount << " nodes and ";
  std::cerr << edgeSource.size() << " links.\n";
  std::cerr << "Reading data complete.\n\n";
  
  return netPointer.release(); // release the pointer so that it is not destroyed
  // (we want to return it)
}

// <---------------- readNet4 --------------------------













                                                              
/*  function outputEdgesAndWeights(NetType& theNet)                             
    Prints to std::cout the edges and weights of the network in the format      
    SOURCE DEST WEIGHT                                                          
*/                                                                              

template<typename NetType>                                                      
void outputEdgesAndWeights(NetType& theNet) {                                   
  
  // Print out edges and weights of the network (source dest weight):           
  for (size_t i=0; i<theNet.size(); ++i) {                                      
    for (typename NetType::const_edge_iterator j=theNet(i).begin();             
         !j.finished(); ++j) {                                                  
      if (i<(*j))                                                               
        std::cout << i <<  "\t" << (*j) << "\t" << j.value() << "\n";       
      //  std::cout << i+1 <<  "\t" << (*j)+1 << "\t" << j.value() << "\n";       
      //printing nodal indices as  1...N instead of 0...N-1                     
    }                                                                           
  }                                                                             
}                                                                               
// <--- outputEdgesAndWeights                                                   
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -           












/*  function outputEdges(NetType& theNet)                                       
    Prints to std::cout the edges of the network in the format                  
    SOURCE DEST                                                                 
*/

template<typename NetType>
void outputEdges(NetType& theNet) {

  // Print out edges of the network (source dest):                              
  for (size_t i=0; i<theNet.size(); ++i) {
    for (typename NetType::const_edge_iterator j=theNet(i).begin();
         !j.finished(); ++j) {
      if (i<(*j))
        std::cout << i <<  "\t" << (*j) << "\t" << "\n";
      //std::cout << i+1 <<  "\t" << (*j)+1 << "\t" << "\n";
      //printing nodal indices as  1...N instead of 0...N-1                     
    }
  }
}
// <--- outputEdges                                                             
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -           












                                       
/*  function outputEdgesAndWeights2(NetType& theNet, const size_t logWeights=0, 
std::ofstream & myfile = std::cout)
    Prints  the edges and weights of the network in the format      
    SOURCE DEST WEIGHT                                                          
*/                                                       
template<typename NetType>                                                      
void outputEdgesAndWeights2(NetType& theNet, const size_t logWeights=0, 
std::ofstream & myfile = std::cout) {                                   
  
  // Print out edges and weights of the network (source dest weight):           
  for (size_t i=0; i<theNet.size(); ++i) {                                      
    for (typename NetType::const_edge_iterator j=theNet(i).begin();             
         !j.finished(); ++j) {                                                  
      if (i<(*j))                                                               
        if (logWeights)                                              
          myfile << i <<  "\t" << (*j) << "\t" << log(j.value()) << "\n";       
        else
          myfile << i <<  "\t" << (*j) << "\t" << j.value() << "\n";   
    }                                                                           
  }                                                                             
}             
// <--- outputEdgesAndWeights2
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -           

















/*function copyNet*/
/* copies net1 to net2, the nets have to be of the same size */

template<typename NetType>
void copyNet(NetType & net1, NetType & net2) {

  assert(net1.size() == net2.size() );
  size_t netSize = net1.size();
  ClearNet(net2, net2.size());

  for (size_t i=0; i<netSize; ++i) {
    for (typename NetType::const_edge_iterator j=net1(i).begin(); !j.finished(); ++j) {
      net2[i][*j] = net1(i)[*j];
    }
  }
}
// <--- copyNet
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  













/* function numberOfEdges*/
/* calculates the number of edges in the network */
template<typename NetType>
size_t numberOfEdges(NetType & net) {
  size_t links=0;

  for (size_t i=0; i<net.size(); ++i) {
    links += net(i).size();
  }
  links=links/2; // divide by two to get the number of links                                            
  return links;
}
// <--- numberOfEdges
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  










/* function numberOfTriangles*/
/* calculates the number of triangles in the network 
   (tested on 3-clique, 4-clique, and chain - gives correct answers 1, 4, 0)
*/
template<typename NetType>
size_t numberOfTriangles(NetType & net) {

  size_t trianglesum=0;
  
  // Go through each node in the network:
  for (size_t i=0; i<net.size(); ++i) {
    
    // Go through the neighbors of node i:
    for (typename NetType::const_edge_iterator j=net(i).begin();
	 !j.finished();
	 ++j) {
      
      for (typename NetType::const_edge_iterator k=net(*j).begin();
	   !k.finished();
	   ++k) {
	if (net(*k)[i] != 0) {
	  trianglesum++;
	}
      }
    }
  }
  
  return trianglesum/6; // each triangle was counted 6 times 
}
// <--- numberOfTriangles
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  














// ConnectivityCheck ---->                                                   
/*                                                                           
                                                                             
The function ConnectivityCheck uses the Dijkstra algorithm                   
to determine whether or not a path exists between the first                  
node in the network and all others - in other words, whether                 
or not the network is connected. In fact, it finds all shortest              
paths from this node, which takes an equivalent time to calculate.           
                                                                             
If the network is connected, all nodes are reachable from                    
any single node. We start the search with the first node                     
in the network, since it always exists for a nonempty network.               
                                                                             
To carry out the Dijkstra algorithm, we use the Dijkstrator                  
from lcelib/nets/Dijkstrator.H.                                
                                                                             
*/

template<typename NetType>
bool ConnectivityCheck(NetType & theNet) {

  /* Find all shortest paths from node 0 */
  Dijkstrator<NetType> paths(theNet,0);
  for (; !paths.finished(); ++paths) {
  }

  /* Check whether all other nodes were reachable */
  if (paths.getFoundSet().size() == theNet.size() )
    return true;
  else if (paths.getFoundSet().size() < theNet.size() )
    return false;
  else
    assert(false);

  return (false); //This should never happen, but not returning a value is illegal.
}
//  <---- ConnectivityCheck                                                  












// ClearNet ---->
/*   A function to clear all edges of a network.                            
     The number of nodes does not change. */

template <typename NetType>
void ClearNet(NetType& theNet, size_t netSize) {
  /* Go through each node */
  for (size_t i=0; i<netSize; ++i) {
    /* and remove the links to its neighbors */
    for (typename NetType::edge_iterator j=theNet[i].begin();
         !j.finished();
         ++j) {
      j.value()=0;  // don't use 'theNet[i][*j]=0;' to avoid messing up the iterator
    }
  }
}
//  <---- ClearNet   











 


/* outputOverlap  (April 4 2006, Riitta) */

/* function outputOverlap 

   calculates the overlap for each edge in the network, defined by

      O_ij = n_ij / ( (k_i-1) + (k_j-1) - n_ij ), 

   where n_ij is the number of common neighbors of i and j
   and k_i is the degree of i,

   and outputs the results to std::cout in the usual format
      i j overlap 
   (each edge on its own row).

   If the input integerOutput=true, each overlap value is multiplied
   by 10 000 and rounded to integers before printing (integer values
   instead of decimals make it safer to sort files by overlap using
   the shell command sort)

   This function was tested on a small network generated by hand and
   on a fully connected network.

   If printAverage is set to true, the last line of the output will
   read "AVG <O>". (Added by Lauri Kovanen, 25.5.2007)
*/
template<typename NetType>
void outputOverlap(NetType & net, bool integerOutput=false, bool printAverage = false) {

  size_t ki=0;
  size_t kj=0;
  size_t nij=0;
  double overlap=0;
  double overlap_avg=0;
  size_t nan_count=0;

  // Go through each node in the network:                                                  
  for (size_t i=0; i<net.size(); ++i) {
    ki=net(i).size();
    // Go through the neighbors of node i:                                                 
    for (typename NetType::const_edge_iterator j=net(i).begin();
         !j.finished();
         ++j) {
      if ( i < (*j) ) { // treat each edge only once                                       
        kj=net(*j).size();
        // count the common neighbors of i and j                                           
        nij=0;
        for (typename NetType::const_edge_iterator k=net(*j).begin();
             !k.finished();
             ++k) {
          if (net(*k)[i] != 0) {
            nij++;
          }
        }
	// calculate and print out the overlap                                                 
	overlap= (double) nij / ( (double) (ki - 1 + kj - 1 - nij) );
	isnan(overlap) ? nan_count++ : overlap_avg += overlap;
	if (integerOutput==false) {
	  std::cout << i <<   " " << (*j) <<  " " << overlap <<  "\n"; 
	}
	else {
          std::cout <<  i <<   " " << (*j) <<  " " << ( round(10000*overlap) ) <<  "\n";
	}
      }
    }
  }

  if (printAverage)
      std::cout << overlap_avg/(numberOfEdges(net)-nan_count) << "\n";
}


// <--- outputOverlap
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  



/* pearsonCoeff  (Jussi) */

/* function pearsonCoeff

Calculates the Pearson correlation coefficient of the degrees
of neighbouring nodes.

*/



template <typename NetType>
float pearsonCoeff(NetType & net) {

  const size_t netSize = net.size();
  float r = 0;
  size_t term1=0, term2=0, term3=0;
  size_t curr_deg, neigh_deg, linkNum = 0;

  // Go through each node in the network:
  for (size_t i=0; i<netSize; ++i) {

    curr_deg=net(i).size(); // Read the degree of node i

    // Go through the neighbors of node i:
    for (typename NetType::const_edge_iterator j=net(i).begin(); !j.finished(); ++j) {
      if ( *j > i ) {
        ++linkNum;
        neigh_deg = net(*j).size();
        term1 += curr_deg * neigh_deg;
        term2 += curr_deg + neigh_deg;
        term3 += curr_deg*curr_deg + neigh_deg*neigh_deg;
      }
    }
  }

  float t1, t2, t3;
  t1 = term1/((float) linkNum);
  t2 = 0.5*term2/((float) linkNum);
  t2 = t2*t2;
  t3 = 0.5*term3/((float) linkNum);

  if (t3-t2 != 0)
    r = (t1-t2)/(t3-t2);
  else
    r = 0;

  return r;
}

// <--- pearsonCoeff
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  


/* pearsonCoeff2  (original by Jussi, modified by Lauri) */

/* function pearsonCoeff2

Calculates the Pearson correlation coefficient of the degrees of
neighbouring nodes. Because the original pearsonCoeff runs into
overflow problems with large networks, this function uses double
precision numbers to expand the range. The result is that the
calculation is no more perfectly accurate, but no overflow _should_
occur for any reasonably sized network.

*/

template <typename NetType>
float pearsonCoeff2(NetType & net) {

  const size_t netSize = net.size();
  float r = 0;
  double term1=0, term2=0, term3=0;
  size_t curr_deg, neigh_deg, linkNum = 0;

  // Go through each node in the network:
  for (size_t i=0; i<netSize; ++i) {

    curr_deg=net(i).size(); // Read the degree of node i

    // Go through the neighbors of node i:
    for (typename NetType::const_edge_iterator j=net(i).begin(); !j.finished(); ++j) {
      if ( *j > i ) {
        ++linkNum;
        neigh_deg = net(*j).size();
        term1 += curr_deg * neigh_deg;
        term2 += curr_deg + neigh_deg;
        term3 += curr_deg*curr_deg + neigh_deg*neigh_deg;
      }
    }
  }

  // DEBUG
  //std::cerr << "term1 = " << term1 << "\nterm2 = " << term2 << "\nterm3 = " << term3 << "\n";

  double t1, t2, t3;
  t1 = term1/linkNum;
  t2 = 0.5*(term2/linkNum);
  t2 = t2*t2;
  t3 = 0.5*(term3/linkNum);

  //std::cerr << "t1 = " << t1 << ", t2 = " << t2 << ", t3 = " << t3 << "\n";

  if (t3-t2 != 0)
    r = (t1-t2)/(t3-t2);
  else
    r = 0;

  //std::cerr << "r =  " << r << "\n";

  return r;
}

// <--- pearsonCoeff2
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  


/*
 function double clustering(typename NetType net, size_t i) 
 Calculates the unweighted clustering coefficient for a 
 given node in the given network.

 Returns -1 if the degree of the node is less than 2, 
 since in this case clustering is not defined for it.

 Tested by hand with small networks. 

*/

template<typename NetType>
double clustering(NetType & net, const size_t i) {
  
  if ( (i<0) ||  ( i>=net.size() ) ) {std::cerr << "In the function calculating clustering: Node index exceeds network size or is negative \n\n"; exit(-1); } 

  size_t k=net(i).size(); // node degree
  if (k<2) return -1; /* clustering not defined for degree > 2 , returning -1 
			 (careful when calculating averages!!!) */
  else {
    size_t trianglesum_x2=0;
    // Go through the neighbors of node i:
    for (typename NetType::const_edge_iterator j=net(i).begin(); !j.finished(); ++j) {
      // ...and check if some of their neighbors are also neighbors of node i.    
      // If yes, increase the triangle counter. 
      for (typename NetType::const_edge_iterator k=net(*j).begin(); !k.finished(); ++k) {
	if (net(*k)[i] != 0) {
	  trianglesum_x2++;
	}
      }
    }
    return (double) trianglesum_x2 / k / (k-1); 
  }
}
// <--- clustering
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  













/* shuffle   (Sept 14 2006, Riitta) */

/*
Shuffles a given vector in linear time (produces a random permutation).

The algorithm: 
Go through the elements in order from index 0 to N-1. Swap the element 
in place i with one of the remaining elements (in places (i+1)...N). 
This produces a uniform distribution. 
(Note that swapping each element with an element in ANY of the N places 
does NOT produce a uniform distribution.)

Tested with small vectors (~ 10 elements) that the probability of 
element i ending up in place j is 1/N. 
Sep 13 2006, Riitta
*/


template<typename elementType, typename Generator>
void shuffle(std::vector<elementType> & order, Generator & generator) {
  
  size_t N=order.size();
  for (size_t i=0; i<N; ++i) { 
    /* this is the old way which produces biased results: 
    size_t toss=generator.next(N); // an index uniformly randomly from [0,N)    */
    /* and here's the new, correct way: */
    size_t toss=generator.next(N-i)+i; // an index uniformly randomly from [i,N)
    // std::cerr << "tossed index from " << 0+i << "..." << N-1 << "\n";
    // swap i'th element with the tossed element
    size_t temp=order.at(i);
    order.at(i)=order.at(toss);
    order.at(toss)=temp;
  }
  
  // print out the order for testing:
  //  for  (size_t i=0; i<order.size(); ++i) {  std::cerr << order.at(i)  << "\n"; };
}
// <--- shuffle
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 







/*  double overlap(NetType & net, size_t i, size_t j) 
Calculates the overlap for a given edge in the given network. 
Tested by hand for a small network (see overlapTest.cpp
for a program that reads in a net and outputs the overlap
for each edge, but tested nets not included in the file)

April 26 2006, Riitta
*/

template <typename NetType>
double overlap(NetType & net, size_t i, size_t j) {
  assert( i < net.size() );
  assert( j < net.size() );
  size_t ki=net(i).size();
  size_t kj=net(j).size();
  if ( (ki < 2) && (kj < 2) ) { 
    std::cerr << "Overlap for edge " << i << "-" << j << " not defined, returning -1. Careful with averaging!\n";
    return (double) -1;
  }
  else {
    // count the common neighbors of i and j                                           
    size_t nij=0;
    for (typename NetType::const_edge_iterator k=net(j).begin();
	 !k.finished();
	 ++k) {
      if (net(*k)[i] != 0) {
	nij++;
      }
    }
    // calculate and return the overlap 
    double overlap_ij = (double) nij / ( (double) (ki - 1 + kj - 1 - nij) );
    return overlap_ij;
  }
}


template<typename NetType>
NetType * findLargestComponent(NetType & net)
{
  
  // find the largest component
  const size_t netSize = net.size();
  KruskalTree<true> kpuu(netSize);
  for (size_t source=0; source<netSize; ++source) {
    for (typename NetType::const_edge_iterator target=net(source).begin(); !target.finished(); ++target) {
      if ( *target > source) 	kpuu.addEdge(source,*target);
    }
  }
  // std::cerr << kpuu.getClusterID(10) << "\t" <<  kpuu.getClusterSize(10) << "\n";
  size_t giantSize = 0;
  size_t currentClusterSize;
  size_t maxClusterIndex = 0;
  size_t node = 0;
  while (giantSize < netSize/2 && node < netSize) {
    currentClusterSize = kpuu.getClusterSize( node );
    if ( currentClusterSize > giantSize ) {
      maxClusterIndex = kpuu.getClusterID( node );
      giantSize = currentClusterSize;
    }
    ++node;
  }
  if ( giantSize <= netSize/2 ) std::cerr << "Largest component less than half of the whole network!\n";
  else std::cerr << "Largest component size is: " << giantSize << "\n";
  
  // Construct the net.
  std::auto_ptr<NetType> netPointer(new NetType(giantSize));
  NetType& net2 = *netPointer;  // Create a reference for easier access.

  size_t newIndexes[netSize];
  size_t counter=0;
  // 
  for (size_t source=0; source<netSize; ++source) {
    if ( kpuu.getClusterID(source) == maxClusterIndex ) {
      newIndexes[source] = counter;                        // rename nodes which belong to giant component
      counter++;
    }
  }
  
  // counter should be now giantSize;
  if (counter != giantSize) std::cerr << "Something wrong with largest component!\n";
  for (size_t source=0; source<netSize; ++source) {
    if ( kpuu.getClusterID(source) == maxClusterIndex ) {
      for (typename NetType::const_edge_iterator target=net(source).begin(); !target.finished(); ++target) {
	if ( kpuu.getClusterID(*target) == maxClusterIndex ) {
	  net2[newIndexes[source]][newIndexes[*target]] = net(source)[*target]; // these nodes belong to same cluster
	}
      }
    }
  }

  return netPointer.release(); // release the pointer so that it is not destroyed

}

/* collapseIndices

Collapses the indices of a network so that they run from 0 to N, where
N is the number of links in the network.
*/

template<typename NetType>
NetType * collapseIndices(NetType & net)
{
  std::map<size_t, size_t> nodeIDmap;
  size_t counter = 0;

  // Create a map of nodes that really exist.
  for (size_t i = 0; i < net.size(); i++)
    {
      if ( net(i).size() > 0)
	{
	  nodeIDmap[i] = counter;
	  counter++;
	}
    }

  // Construct the net.
  std::auto_ptr<NetType> netPointer(new NetType(nodeIDmap.size()));
  NetType& net2 = *netPointer;  // Create a reference for easier access.

  for (size_t i = 0; i < net.size(); i++)
    {
      for (typename NetType::const_edge_iterator j = net(i).begin(); !j.finished(); ++j)
	{
	  net2[nodeIDmap[i]][nodeIDmap[*j]] = net(i)[*j];
	}
    }
 
  return netPointer.release(); // release the pointer so that it is not destroyed
}


template<typename NetType>
float clusteringCoefficient(NetType & net, size_t i) {

size_t sum=0;
// naapurit:
for (typename NetType::const_edge_iterator j=net(i).begin();
     !j.finished();
     ++j) {
   // niiden naapurit. Huomaa, ettei sä
  for (typename NetType::const_edge_iterator k=net(*j).begin();
     !k.finished();
     ++k) {
     if (net(*k)[i] != 0) sum++;
}
}
 float result; 
 if(net(i).size()>1){
   result=((float) sum)/net(i).size() / ( net(i).size()-1 );
 }else{
   result=0;
 }

return result;
 
}

template<typename NetType>
float clusteringCoefficient_Onnela(NetType & net, size_t i) {

float sum=0;
// naapurit:
for (typename NetType::const_edge_iterator j=net(i).begin();
     !j.finished();
     ++j) {
   // niiden naapurit. Huomaa, ettei sä
  for (typename NetType::const_edge_iterator k=net(*j).begin();
     !k.finished();
     ++k) {
    if (net(*k)[i] != 0){
      sum=sum+pow(net(*k)[i]*(k.value())*(j.value()),(float)1.0/3);
    }
  }
}
 float result;
 if(net(i).size()>1){
   result=((float) sum)/net(i).size() / ( net(i).size()-1 );
 } else {
   result=0;
 }
return result; 
}



template<typename NetType>
float clusteringCoefficient_Barrat(NetType & net, size_t i) {

  float sum=0;
  float strength=0;
// naapurit:
for (typename NetType::const_edge_iterator j=net(i).begin();
     !j.finished();
     ++j) {
  strength=strength+j.value();
   // niiden naapurit. Huomaa, ettei sä
  for (typename NetType::const_edge_iterator k=net(*j).begin();
     !k.finished();
     ++k) {
    if (net(*k)[i] != 0){
      sum=sum+(net(*k)[i]+(j.value()))/2;
    }
  }
}
 float result;
 if (strength!=0 & net(i).size()>1){
   result=((float) sum)/strength / ( net(i).size()-1 );
 } else {
   result=0;
 }

return result; 
}


template<typename NetType>
float clusteringCoefficient_Ahnert(NetType & net, size_t i) {

  float sum=0;
  float alakerta=0;
// naapurit:
for (typename NetType::const_edge_iterator j=net(i).begin();
     !j.finished();
     ++j) {
   // niiden naapurit. Huomaa, ettei sä
  for (typename NetType::const_edge_iterator k=net(*j).begin();
     !k.finished();
     ++k) {
    if (net(*k)[i] != 0){
      sum=sum+net(*k)[i]*(k.value())*(j.value());
    }
  }

  for (typename NetType::const_edge_iterator k=net(i).begin();
       !k.finished();
       ++k){
    if(j!=k){
      alakerta=alakerta+(j.value())*(k.value());
    } 
  }

}

 float result;
 if(alakerta!=0){
   result=((float) sum)/alakerta;
 }else{
   result=0;
 }
return result; 
}


#endif //~ NETEXTRAS_H 
