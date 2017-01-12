// File: community.h
// -- community detection source file
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

#include "community.h"


using namespace std;

Community::Community(char * filename, int type, int nbp, double minm) {
  g = Graph(filename, type);
  size = g.nb_nodes;

  n2c.resize(size);
  in.resize(size);
  tot.resize(size);

  for (int i=0 ; i<size ; i++) {
    n2c[i] = i;
    in[i]  = g.nb_selfloops(i);
    tot[i] = g.weighted_degree(i);
  }

  nb_pass = nbp;
  min_modularity = minm;
}

Community::Community(Graph gc, int nbp, double minm) {
  g = gc;
  size = g.nb_nodes;

  n2c.resize(size);
  in.resize(size);
  tot.resize(size);

  for (int i=0 ; i<size ; i++) {
    n2c[i] = i;
    in[i]  = g.nb_selfloops(i);
    tot[i] = g.weighted_degree(i);
  }

  nb_pass = nbp;
  min_modularity = minm;
}

void
Community::display() {
  //cerr << endl << "<" ;
 // for (int i=0 ; i<size ; i++)
    //cerr << " " << i << "/" << n2c[i] << "/" << in[i] << "/" << tot[i] ;
  //cerr << ">" << endl;
}


double
Community::modularity() {
  double q  = 0.;
  double m2 = (double)g.total_weight;

  for (int i=0 ; i<size ; i++) {
    if (tot[i]>0)
      q += (double)in[i]/m2 - ((double)tot[i]/m2)*((double)tot[i]/m2);
  }

  return q;
}


map<int,int>
Community::neigh_comm(int node) {
  map<int,int> res;
  pair<int *,int *> p = g.neighbors(node);

  int deg = g.nb_neighbors(node);

  res.insert(make_pair(n2c[node],0));

  for (int i=0 ; i<deg ; i++) {
    int neigh        = *(p.first+i);
    int neigh_comm   = n2c[neigh];
    int neigh_weight = (g.weights==NULL)?1:*(p.second+i);
    
    if (neigh!=node) {
      map<int,int>::iterator it = res.find(neigh_comm);
      if (it!=res.end())
	it->second+=neigh_weight;
      else
	res.insert(make_pair(neigh_comm,neigh_weight));
    }
  }
  
  return res;
}


void
Community::partition2graph() {
  vector<int> renumber(size, -1);
  for (int node=0 ; node<size ; node++) {
    renumber[n2c[node]]++;
  }

  int final=0;
  for (int i=0 ; i<size ; i++)
    if (renumber[i]!=-1)
      renumber[i]=final++;


  for (int i=0 ; i<size ; i++) {
    pair<int *,int *> p = g.neighbors(i);

    int deg = g.nb_neighbors(i);
    for (int j=0 ; j<deg ; j++) {
      int neigh = *(p.first+j);
      cout << renumber[n2c[i]] << " " << renumber[n2c[neigh]] << endl;
    }
  }
}

void
Community::display_partition() {
  vector<int> renumber(size, -1);
  for (int node=0 ; node<size ; node++) {
    renumber[n2c[node]]++;
  }

  int final=0;
  for (int i=0 ; i<size ; i++)
    if (renumber[i]!=-1)
      renumber[i]=final++;

  for (int i=0 ; i<size ; i++)
    cout << i << " " << renumber[n2c[i]] << endl;
}

// This function has to be revisited
// malloc is dirty
Graph
Community::partition2graph_binary() {
  vector<int> renumber(size, -1);
  for (int node=0 ; node<size ; node++) {
    renumber[n2c[node]]++;
  }

  int final=0;
  for (int i=0 ; i<size ; i++)
    if (renumber[i]!=-1)
      renumber[i]=final++;

  // Compute communities
  vector<vector<int> > comm_nodes(final);
  for (int node=0 ; node<size ; node++) {
    comm_nodes[renumber[n2c[node]]].push_back(node);
  }

  // unweigthed to weighted
  Graph g2;
  g2.nb_nodes = comm_nodes.size();
  g2.degrees  = (int *)malloc(comm_nodes.size()*4);
  g2.links    = (int *)malloc((long)10000000*8);
  g2.weights  = (int *)malloc((long)10000000*8);

  long where = 0;
  int comm_deg = comm_nodes.size();
  for (int comm=0 ; comm<comm_deg ; comm++) {
    map<int,int> m;
    map<int,int>::iterator it;

    int comm_size = comm_nodes[comm].size();
    for (int node=0 ; node<comm_size ; node++) {
      pair<int *,int *> p = g.neighbors(comm_nodes[comm][node]);
      int deg = g.nb_neighbors(comm_nodes[comm][node]);
      for (int i=0 ; i<deg ; i++) {
	int neigh        = *(p.first+i);
	int neigh_comm   = renumber[n2c[neigh]];
	int neigh_weight = (g.weights==NULL)?1:*(p.second+i);

	it = m.find(neigh_comm);
	if (it==m.end())
	  m.insert(make_pair(neigh_comm, neigh_weight));
	else
	  it->second+=neigh_weight;
      }
    }

    g2.degrees[comm]=(comm==0)?m.size():g2.degrees[comm-1]+m.size();
    g2.nb_links+=m.size();

    for (it = m.begin() ; it!=m.end() ; it++) {
      g2.total_weight  += it->second;
      g2.links[where]   = it->first;
      g2.weights[where] = it->second;
      where++;
    }
  }

  realloc(g.links, (long)g2.nb_links*4);
  realloc(g.weights, (long)g2.nb_links*4);

  return g2;
}

double
Community::one_level() {
  bool improvement = false;
  int nb_pass_done = 0;
  double new_mod   = modularity();
  double cur_mod   = new_mod;

  // repeat while 
  //   there is an improvement of modularity
  //   or there is an improvement of modularity greater than a given epsilon 
  //   or a predefined number of pass have been done

  vector<int> random_order(size);
  for (int i=0 ; i<size ; i++)
    random_order[i]=i;
  for (int i=0 ; i<size-1 ; i++) {
    int rand_pos = rand()%(size-i)+i;
    int tmp      = random_order[i];
    random_order[i] = random_order[rand_pos];
    random_order[rand_pos] = tmp;
  }

  do {
    cur_mod = new_mod;
    improvement = false;
    nb_pass_done++;

    // for each node: remove the node from its community and insert it in the best community
    for (int node_tmp=0 ; node_tmp<size ; node_tmp++) {
      int node = node_tmp;
//      int node = random_order[node_tmp];
//      if (node%1000000==0) {fprintf(stderr,"%d ",node); fflush(stderr);}
      int node_comm     = n2c[node];

      // computation of all neighboring communities of current node
      map<int,int> ncomm   = neigh_comm(node);

      // remove node from its current community
      remove(node, node_comm, ncomm.find(node_comm)->second);

      // compute the nearest community for node
      // default choice for future insertion is the former community
      int best_comm        = node_comm;
      int best_nblinks     = 0;//ncomm.find(node_comm)->second;
      double best_increase = 0.;//modularity_gain(node, best_comm, best_nblinks);
      for (map<int,int>::iterator it=ncomm.begin() ; it!=ncomm.end() ; it++) {
        double increase = modularity_gain(node, it->first, it->second);
        if (increase>best_increase) {
          best_comm     = it->first;
          best_nblinks  = it->second;
          best_increase = increase;
        }
      }

      // insert node in the nearest community
      //      //cerr << "insert " << node << " in " << best_comm << " " << best_increase << endl;
      insert(node, best_comm, best_nblinks);
     
      if (best_comm!=node_comm)
        improvement=true;
    }

    new_mod = modularity();
    ////cerr << "pass number " << nb_pass_done << " of " << nb_pass 
         //<< " : " << new_mod << " " << cur_mod << endl;

  } while (improvement && new_mod-cur_mod>min_modularity && nb_pass_done!=nb_pass);

  return new_mod;
}

