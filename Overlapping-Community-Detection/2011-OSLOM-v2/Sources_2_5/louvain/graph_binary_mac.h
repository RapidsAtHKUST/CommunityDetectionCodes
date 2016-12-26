// File: graph_binary.h
// -- graph handling header file
//-----------------------------------------------------------------------------
// Community detection 
// Based on the article "Fast unfolding of community hierarchies in large networks"
// Copyright (C) 2008 V. Blondel, J.-L. Guillaume, R. Lambiotte, E. Lefebvre
//
// This program must not be distributed without agreement of the above mentionned authors.
//-----------------------------------------------------------------------------
// Author   : E. Lefebvre, adapted by J.-L. Guillaume
// Email    : jean-loup.guillaume@lip6.fr
// Location : Paris, France
// Time	    : February 2008
//-----------------------------------------------------------------------------
// see readme.txt for more details

#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include </usr/include/sys/malloc.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>

#define WEIGHTED   0
#define UNWEIGHTED 1

using namespace std;

class Graph {
 public:
  int nb_nodes;
  int nb_links;
  int total_weight;  

  int *degrees;
  int *links;
  int *weights;

  Graph();

  // binary file format is
  // 4 bytes for the number of nodes in the graph
  // 4*(nb_nodes) bytes for the cumulative degree for each node:
  //    deg(0)=degrees[0]
  //    deg(k)=degrees[k]-degrees[k-1]
  // 4*(sum_degrees) bytes for the links
  // IF WEIGHTED 4*(sum_degrees) bytes for the weights
  Graph(char *filename, int type);
  
  Graph(int nb_nodes, int nb_links, int total_weight, int *degrees, int *links, int *weights);
  
  Graph(int n1, int k1, int n2, int k2, int n3, int k3);
  Graph(int n1, int k1, int n2, int k2);
  
  void display(void);
  void display_binary(char *outfile);

  // return the number of neighbors (degree) of the node
  inline int nb_neighbors(int node);

  // return the number of self loops of the node
  inline int nb_selfloops(int node);

  // return the weighted degree of the node
  inline int weighted_degree(int node);

  // return pointers to the first neighbor and first weight of the node
  inline pair<int *, int *> neighbors(int node);
};


inline int
Graph::nb_neighbors(int node) {
  assert(node>=0 && node<nb_nodes);

  if (node==0)
    return degrees[0];
  else
    return degrees[node]-degrees[node-1];
}

inline int
Graph::nb_selfloops(int node) {
  assert(node>=0 && node<nb_nodes);

  pair<int *,int *> p = neighbors(node);
  for (int i=0 ; i<nb_neighbors(node) ; i++) {
    if (*(p.first+i)==node) {
      if (weights!=NULL)
	return *(p.second+i);
      else 
	return 1;
    }
  }
  return 0;
}

inline int
Graph::weighted_degree(int node) {
   assert(node>=0 && node<nb_nodes);

   pair<int *,int *> p = neighbors(node);
   if (p.second==NULL)
     return nb_neighbors(node);
   else {
     int res = 0;
     for (int i=0 ; i<nb_neighbors(node) ; i++)
       res += *(p.second+i);
     return res;
   }
}

inline pair<int *,int *>
Graph::neighbors(int node) {
  assert(node>=0 && node<nb_nodes);

  if (node==0)
    return make_pair(links, weights);
  else if (weights!=NULL)
    return make_pair(links+(long)degrees[node-1], weights+(long)degrees[node-1]);
  else
    return make_pair(links+(long)degrees[node-1], weights);
}


#endif // GRAPH_H
