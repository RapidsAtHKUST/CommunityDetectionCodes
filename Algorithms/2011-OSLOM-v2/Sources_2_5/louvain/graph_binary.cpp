// File: graph_binary.cpp
// -- graph handling source
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

#include <sys/mman.h>
#include "graph_binary.h"
#include <unistd.h>


Graph::Graph() {
  nb_nodes     = 0;
  nb_links     = 0;
  total_weight = 0;
}

Graph::Graph(char *filename, int type) {
  ifstream finput;
  finput.open(filename,fstream::in | fstream::binary);

  // read number of nodes on 4 bytes
  finput.read((char *)&nb_nodes, 4);

  // read cumulative degree sequence: 4 bytes for each node
  // cum_degree[0]=degree(0); cum_degree[1]=degree(0)+degree(1), etc.

  degrees = (int *)malloc((long)nb_nodes*4);
  finput.read((char *)degrees, (long)nb_nodes*4);

  // read links: 4 bytes for each link (each link is counted twice)
  nb_links=degrees[nb_nodes-1]/2;
  links = (int *)malloc((long)nb_links*8);
  finput.read((char *)links, (long)nb_links*8);  
  //cerr << "total : " << nb_links << endl;

  // IF WEIGHTED : read weights: 4 bytes for each link (each link is counted twice)
  if (type==WEIGHTED) {
    weights = (int *)malloc((long)nb_links*8);
    finput.read((char *)weights, (long)nb_links*8);  
    total_weight=0;
    for (int i = 0 ; i<nb_links*2 ; i++) {
      total_weight += weights[i];
    }
  } else {
    weights = NULL;
    total_weight = 2*nb_links;
  }
}

// generates a random graph using the benchmark approach
Graph::Graph(int n1, int k1, int n2, int k2, int n3, int k3) {
  srand(time(NULL));
  nb_nodes = n1*n2*n3;

  vector<vector<int> > gr(nb_nodes);
  for (int i=0 ; i<nb_nodes ; i++)
    gr[i].resize(nb_nodes,0);

//  cerr << (k1*1.)/(n1*1.) << " " 
//       << (k2*1.)/(n1*n2*1.) << " " 
//       << (k3*1.)/(n1*n2*n3*1.) << endl;

  nb_links = 0;
  for (int i=0 ; i<nb_nodes ; i++)
    for (int j=i+1 ; j<nb_nodes ; j++) {
      double v = rand()*1./RAND_MAX;
      if (i/n1==j/n1) { // i and j in the same subgroup
//	cout << i << " " << j << " 1 : " << v << " " << (k1*1.)/(n1-1.) ;
	if (v<=(k1*1.)/(n1-1.)) {
	  gr[i][j]=gr[j][i]=1;
	  nb_links++;
//	  cout << " : ok" ;
	}
//	cout << endl;
      } else if (i/(n1*n2)==j/(n1*n2)) { // i and j in the same group
//	cout << i << " " << j << " 2 : " << v << " " << (k2*1.)/(n1*(n2-1.)) ;
	if (v<=(k2*1.)/(n1*(n2-1.))) {
	  gr[i][j]=gr[j][i]=1;
	  nb_links++;
//	  cout << " : ok" ;
	}
//	cout << endl;
      } else { // i and j in different groups
//	cout << i << " " << j << " 3 : " << v << " " << (k3*1.)/(n1*n2*(n3-1.)) ;
	if (v<=(k3*1.)/(n1*n2*(n3-1.))) {
	  gr[i][j]=gr[j][i]=1;
	  nb_links++;
//	  cout << " : ok" ;
	}
//	cout << endl;
      }
    }

//  cerr << nb_links << endl;
  
  total_weight = 2*nb_links;
  weights      = NULL;

  degrees = (int *)malloc((long)nb_nodes*4);
  for (int i=0 ; i<nb_nodes ; i++) {
    int d = 0;
    for (int j=0 ; j<nb_nodes ; j++)
      d+=gr[i][j];
    degrees[i]=d;
  }
  for (int i=1 ; i<nb_nodes ; i++)
    degrees[i]+=degrees[i-1];

  links = (int *)malloc((long)nb_links*8);
  int pos=0;
  for (int i=0 ; i<nb_nodes ; i++)
    for (int j=0 ; j<nb_nodes ; j++)
      if (gr[i][j]==1)
	links[pos++]=j;

//  for (int i=0 ; i<nb_nodes ; i++)
//    cerr << degrees[i] << " " ;
//  cerr << endl;
}

// generates a random graph using the benchmark approach
Graph::Graph(int n1, int k1, int n2, int k2) {
  srand(getpid());

//  srand(time(NULL));
  nb_nodes = n1*n2;

  vector<vector<int> > gr(nb_nodes);
  for (int i=0 ; i<nb_nodes ; i++)
    gr[i].resize(nb_nodes,0);

  nb_links = 0;
  for (int i=0 ; i<nb_nodes ; i++)
    for (int j=i+1 ; j<nb_nodes ; j++) {
      double v = rand()*1./RAND_MAX;
      if (i/n1==j/n1) { // i and j in the same subgroup
	if (v<=(k1*1.)/(n1-1.)) {
	  gr[i][j]=gr[j][i]=1;
	  nb_links++;
	}
      } else { // i and j in different groups
	if (v<=(k2*1.)/(n1*(n2-1.))) {
	  gr[i][j]=gr[j][i]=1;
	  nb_links++;
	}
      }
    }

  total_weight = 2*nb_links;
  weights      = NULL;

  degrees = (int *)malloc((long)nb_nodes*4);
  for (int i=0 ; i<nb_nodes ; i++) {
    int d = 0;
    for (int j=0 ; j<nb_nodes ; j++)
      d+=gr[i][j];
    degrees[i]=d;
  }
  for (int i=1 ; i<nb_nodes ; i++)
    degrees[i]+=degrees[i-1];

  links = (int *)malloc((long)nb_links*8);
  int pos=0;
  for (int i=0 ; i<nb_nodes ; i++)
    for (int j=0 ; j<nb_nodes ; j++)
      if (gr[i][j]==1)
	links[pos++]=j;
}

Graph::Graph(int n, int m, int t, int *d, int *l, int *w) {
  nb_nodes     = n;
  nb_links     = m;
  total_weight = t;
  degrees      = d;
  links        = l;
  weights      = w;
}


void
Graph::display() {
  for (int node=0 ; node<nb_nodes ; node++) {
    pair<int *,int *> p = neighbors(node);
    for (int i=0 ; i<nb_neighbors(node) ; i++) {
      if (weights!=NULL)
	cout << node << " " << *(p.first+i) << " " << *(p.second+i) << endl;
      else {
		cout << (node+1) << " " << (*(p.first+i)+1) << endl;
	//	cout << (node) << " " << (*(p.first+i)) << endl;
      }
    }   
  }
}

void
Graph::display_binary(char *outfile) {
  ofstream foutput;
  foutput.open(outfile ,fstream::out | fstream::binary);

  foutput.write((char *)(&nb_nodes),4);
  foutput.write((char *)(degrees),4*nb_nodes);
  foutput.write((char *)(links),8*nb_links);
}
