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

// lcelib/nets/models/Marsili.H
// 
// Functions for generation of Marsili networks
//
// (Matteo Marsili, Fernando Vega-Redondo, Frantisek Slanina:
//  The rise and fall of a networked society: A formal model
//  PNAS, Feb 10 2004, vol 101, no. 6, 1439-1442)



/*
 Usage
 -----

 ./Marsili 73783234 > network.edg

*/




#ifndef MARSILI_H
#define MARSILI_H
#include <iostream>
#include <cmath>
#include <string>
#include <list>
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include "../NetExtras.H"  // required for clearNet()
#include "../../misc/BinomialSampler.H"

typedef Set<size_t> nodeSet;

/* The struct into which command line arguments will be read */

struct MarsiliArgs 
{
  size_t netSize;
  float lambda;
  float eta;
  float xi;
  int randseed;
  size_t iter_max;
};

// readMarsiliArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void readMarsiliArgs(struct MarsiliArgs & args, int argc, char** argv)
{
  size_t bookmark = 1;
  if ( (size_t) argc < (5+1)) {  // 5 arguments required: N, p, randseed
    std::cerr << "\nPlease specify arguments: \n\tN, lambda, eta, xi, randseed[, max_iter]\n\n"; exit(1);
  }  
  args.netSize = atoi(argv[bookmark]);  bookmark++;
  args.lambda = atof(argv[bookmark]); bookmark++;
  args.eta = atof(argv[bookmark]); bookmark++;
  args.xi = atof(argv[bookmark]); bookmark++;
  args.randseed = atoi(argv[bookmark]);  bookmark++;
  args.iter_max = 0;
  if ( (size_t) argc == 7) 
    {
      args.iter_max = atoi(argv[bookmark]);
    }  
}  
// <--- readMarsiliArgs



// outputMarsiliArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void outputMarsiliArgs(struct MarsiliArgs & args) 
{
  std::cerr << "Parameters given for simulating Marsili network:\n";
  std::cerr << "(arguments required: N, lambda, eta, xi, randseed)\n\n";
  std::cerr << "Network size:\t\t\t\t\t" << args.netSize << "\n"; 
  std::cerr << "lambda (probability of removing an edge):\t\t" << (float) args.lambda << "\n";
  std::cerr << "eta (probability of adding an edge):\t\t\t" << (float) args.eta << "\n";
  std::cerr << "xi (probability of neighbourhood search):\t\t" << (float) args.xi << "\n";

  std::cerr << "Number of iterations: \t\t\t\t" << args.iter_max << "\n";
  std::cerr << "Random number generator seed (integer): \t" << args.randseed << "\n";
  std::cerr << "\n\n";
} 
// <--- outputMarsiliArgs
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 



// Marsili ---->
/*  
    Generates an unweighted Marsili network. The network is initially an
    empty, fixed size network and is constructed by iterating the
    following rules
 
    (i) For each node, create a random edge to another node with probability eta.

    (ii) With probability xi, each node i asks its random neighbour j to
    introduce it to one of j's random neighbour, k. If link (i,k) already
    exists, nothing happens.

    (iii) For each edge, remove the edge with probability lambda.

    Phases i, ii and iii are done simultaneously.

*/


template<typename NetType, typename Generator>
void Marsili(NetType& net, struct MarsiliArgs & args, Generator & generator)
{
  
  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  NetType net2(args.netSize); // initialize empty helper network

  // Keep count of the edges in the network. We start with an empty net.
  size_t net_edges = 0, net2_edges = 0;

  // If iter_max was not given as input parameter, check every
  // check_period iterations if the number of added edges (edges -
  // edges_old) is less than 1% of the number of edges now. When this
  // happens, do twice the number of iterations done by then and
  // consider it done.
  size_t check_period = 500;
  size_t edges_old = 0;
  bool end_iter = true;
  //std::cerr << "iter_max = " << args.iter_max << "\n";
  if (args.iter_max == 0)
    {
	   end_iter = false;
	   args.iter_max = 20000;
    }

  //Initialize binomial generators.
  BinomialGenerator BinGenEta(args.netSize, args.eta);
  BinomialGenerator BinGenXi(args.netSize, args.xi);

  size_t no_of_links;

  for (size_t main_iter = 1; main_iter <= args.iter_max; main_iter++)
    {
      if (end_iter == false && main_iter % check_period == 0)
	{
	  if (net_edges <= edges_old || (net_edges - edges_old)/(double)net_edges < 0.01)
	    {
	      args.iter_max = 2*main_iter;
	      end_iter = true;
	      //std::cerr << "Now " << main_iter << " iterations, stopping at " << iter_max << "\n";
      	      //std::cerr << "edges = " << edges << ", edges_old = " << edges_old 
	      //          << ", prop = " << (edges - edges_old)/(double)edges << "\n";
	    }
	  
	  edges_old = net_edges;
	}

      /*
      // Print iteration number, <k> and <k²> in standard error every 100 steps.
      if (main_iter % 100 == 0)
	{
	  size_t k2 = 0;
	  for (size_t iter = 0; iter < net.size(); ++iter) k2 += (size_t)pow(net(iter).size(), 2);

	  std::cerr << main_iter << " " << 2*(float)net_edges/(float)args.netSize << " " << (float)k2/(float)args.netSize << "\n";
	}
      */

      // (i)
      // Randomize the number of random links to be added.
      no_of_links = BinGenEta.get(generator.nextNormed());

      //std::cerr << "  (i) Adding " << no_of_links << " random links:";

      // Put links on a set to avoid duplicates.
      nodeSet random_nodes;
      while(random_nodes.size() < no_of_links) {
	random_nodes.put(generator.next(args.netSize));
      }

      // Create random links for nodes in random_nodes.
      for(nodeSet::iterator i_iter = random_nodes.begin(); !i_iter.finished(); ++i_iter) 
	{
	  //std::cerr << " (" << *i_iter << ",";
	  size_t j;
	  do
	    {
	      j = generator.next(args.netSize);
	    } while (*i_iter == j || (net(*i_iter)[j] == 1)); // no loops of length 1, no existing links 
	  
	  //std::cerr << j << ")";

	  if (net2[*i_iter][j] == 0)
	    {
	      net2[*i_iter][j] = 1; // Add link to net2
	      ++net2_edges;
	    }
	}

      //std::cerr << "\n";

      //(ii)

      // Create a link to neighbour of a neighbour
      no_of_links = BinGenXi.get(generator.nextNormed());

      //std::cerr << "  (ii) Adding " << no_of_links << " neighbourhood links.\n";

      // Put links in a set to avoid duplicates
      nodeSet neighbour_nodes;
      while(neighbour_nodes.size() < no_of_links) {
	neighbour_nodes.put(generator.next(args.netSize));
      }

      // Create links to neighbours for nodes in nodeset.
      for(nodeSet::iterator i_iter = neighbour_nodes.begin(); !i_iter.finished(); ++i_iter) 
	{
	  if (net(*i_iter).size() == 0) continue; // No neighbours, continue to next random node.

	  // Select j, a random neighbour of i.
	  size_t neighbour_no = generator.next(net(*i_iter).size());
	  size_t count = 0, j;
	  for (typename NetType::edge_iterator j_iter = net[*i_iter].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if (count == neighbour_no)
		{
		  j = *j_iter;
		  break;
		}
	      count++;
	    }

	  if (net(j).size() == 1) continue; // The only neighbour of j is i, continue to next random node.

	  // Select k, a random neighbour of j, making sure k != i.
	  size_t k;
	  do {
	    size_t neighbour_no = generator.next(net(j).size());
	    size_t count = 0;
	    for (typename NetType::edge_iterator k_iter = net[j].begin(); !k_iter.finished(); ++k_iter)
	      {
		if (count == neighbour_no)
		  {
		    k = *k_iter;
		    break;
		  }
		count++;
	      }
	  } while (k == *i_iter); // no loops of length 1
	    
	  // Add a link between i and k if it does not already exist.
	  if (net[*i_iter][k] == 0 && net2[*i_iter][k] == 0)
	    {
	      net2[*i_iter][k] = 1;
	      ++net2_edges;
	    }
	}


      // (iii)

      // Randomize the number of links to be removed.
      no_of_links = binomial(net_edges, args.lambda, generator.nextNormed());
      //std::cerr << "  (iii) Removing " << no_of_links << " links (from total of " << net_edges << "):";

      // Remove the links.
      for (size_t iter = 0; iter < no_of_links; iter++)
	{
	  // Draw a random node weighed by its degree.
	  size_t i = net.weighedRandSlot(generator);
	  // Draw random link
	  size_t neighbour_no = generator.next(net(i).size());
	  size_t count = 0, j;
	  //std::cerr << "  " << iter << ":" << i << "(" << neighbour_no << "/" << net(i).size() << " = "; 
	  for (typename NetType::edge_iterator j_iter = net[i].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if (count == neighbour_no) 
		{
		  j = *j_iter;
		  break;
		}
	      count++;
	    }
	  //std::cerr << j << ") "; 

	  //Remove link, unless if it will be added again anyway.
	  if (net2[i][j] == 0)
	    {
	      net[i][j] = 0;
	      --net_edges;
	    }
	  //std::cerr << "ok\n"; 
	}
      
      //std::cerr << "\n";

      // copy changes from net2 to net

      for ( size_t i = 0; i < args.netSize; ++i)
	{
	  for (typename NetType::edge_iterator j_iter = net2[i].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if ( *j_iter > i )  // take each link only once
		{
		  net[i][*j_iter] = net[i][*j_iter] + j_iter.value(); // add changes
		  j_iter.value() = 0; // remove edge so that net2 will eventually be empty
		}
	    }
	}


      // Update the number of edges
      net_edges += net2_edges;
      net2_edges = 0;

      //std::cerr << "net has " << net_edges << " (" << numberOfEdges(net) << ") edges.\n";
      //std::cerr << "net2 has " << net2_edges << " (" << numberOfEdges(net2) << ") edges.\n";


      // Just to check that the number of edges is calculated correctly.
      assert(net_edges == numberOfEdges(net));



    } // Main iteration ends
}
// <---- Marsili



// MarsiliCertLink ---->
/*  
   In just like Marsili2, except that MarsiliCertLink add a random
   link to every empty node.
*/
template<typename NetType, typename Generator>
void MarsiliCertLink(NetType& net, struct MarsiliArgs & args, Generator & generator)
{
  
  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  NetType net2(args.netSize); // initialize empty helper network

  // Keep count of the edges in the network. We start with an empty net.
  size_t net_edges = 0, net2_edges = 0;

  // If iter_max was not given as input parameter, check every
  // check_period iterations if the number of added edges (edges -
  // edges_old) is less than 1% of the number of edges now. When this
  // happens, do twice the number of iterations done by then and
  // consider it done.
  size_t check_period = 500;
  size_t edges_old = 0;
  bool end_iter = true;
  //std::cerr << "iter_max = " << args.iter_max << "\n";
  if (args.iter_max == 0)
    {
	   end_iter = false;
	   args.iter_max = 20000;
    }

  //Initialize binomial generators.
  BinomialGenerator BinGenEta(args.netSize, args.eta);
  BinomialGenerator BinGenXi(args.netSize, args.xi);

  for (size_t main_iter = 1; main_iter <= args.iter_max; main_iter++)
    {
      if (end_iter == false && main_iter % check_period == 0)
	{
	  if (net_edges <= edges_old || (net_edges - edges_old)/(double)net_edges < 0.01)
	    {
	      args.iter_max = 2*main_iter;
	      end_iter = true;
	      //std::cerr << "Now " << main_iter << " iterations, stopping at " << iter_max << "\n";
      	      //std::cerr << "edges = " << edges << ", edges_old = " << edges_old 
	      //          << ", prop = " << (edges - edges_old)/(double)edges << "\n";
	    }
	  
	  edges_old = net_edges;
	}

      /*
      // Print iteration number, <k> and <k²> in standard error every 100 steps.
      if (main_iter % 100 == 0)
	{
	  size_t k2 = 0;
	  for (iter = 0; iter < net.size(); ++iter) k2 += (size_t)pow(net(iter).size(), 2);

	  std::cerr << main_iter << " " << 2*(float)net_edges/(float)args.netSize << " " << (float)k2/(float)args.netSize << "\n";
	}
      */   

      //std::cerr << "Iteration " << main_iter << "\n";

      // (ii)
      // Randomize the number of random links to be added.
      size_t no_of_links = BinGenEta.get(generator.nextNormed());

      //std::cerr << "  (ii) Adding " << no_of_links << " random links";

      // Put links on a set to avoid duplicates.
      nodeSet random_nodes;
      while(random_nodes.size() < no_of_links) {
	random_nodes.put(generator.next(args.netSize));
      }

      // Add all empty nodes to the set.
      // (This is the only difference to Marsili2.)
      for(size_t i_iter = 0; i_iter < net.size(); ++i_iter) 
	{
	  if (net(i_iter).size() == 0) random_nodes.put(i_iter);
	}
      //std::cerr << " and " << random_nodes.size()-no_of_links << " certain links:";

      // Create random links for nodes in random_nodes.
      for(nodeSet::iterator i_iter = random_nodes.begin(); !i_iter.finished(); ++i_iter) 
	{
	  //std::cerr << " (" << *i_iter << ",";
	  size_t j;
	  do
	    {
	      j = generator.next(args.netSize);
	    } while (*i_iter == j || (net(*i_iter)[j] == 1)); // no loops of length 1, no existing links 
	  
	  //std::cerr << j << ")";

	  if (net2[*i_iter][j] == 0)
	    {
	      net2[*i_iter][j] = 1; // Add link to net2
	      ++net2_edges;
	    }
	}

      //std::cerr << "\n";

      //(iii)
      // Create a link to neighbour of a neighbour
      no_of_links = BinGenXi.get(generator.nextNormed());

      //std::cerr << "  (iii) Adding " << no_of_links << " neighbourhood links:";

      // Put links in a set to avoid duplicates
      nodeSet neighbour_nodes;
      while(neighbour_nodes.size() < no_of_links) {
	neighbour_nodes.put(generator.next(args.netSize));
      }

      // Create links to neighbours for nodes in nodeset.
      for(nodeSet::iterator i_iter = neighbour_nodes.begin(); !i_iter.finished(); ++i_iter) 
	{
	  if (net(*i_iter).size() == 0) continue; // No neighbours, continue to next random node.

	  // Select j, a random neighbour of i.
	  size_t neighbour_no = generator.next(net(*i_iter).size());
	  size_t count = 0, j;
	  for (typename NetType::edge_iterator j_iter = net[*i_iter].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if (count == neighbour_no)
		{
		  j = *j_iter;
		  break;
		}
	      count++;
	    }

	  if (net(j).size() == 1) continue; // The only neighbour of j is i, continue to next random node.

	  // Select k, a random neighbour of j, making sure k != i.
	  size_t k;
	  do {
	    size_t neighbour_no = generator.next(net(j).size());
	    size_t count = 0;
	    for (typename NetType::edge_iterator k_iter = net[j].begin(); !k_iter.finished(); ++k_iter)
	      {
		if (count == neighbour_no)
		  {
		    k = *k_iter;
		    break;
		  }
		count++;
	      }
	  } while (k == *i_iter); // no loops of length 1
	    
	  // Add a link between i and k if it does not already exist.
	  if (net[*i_iter][k] == 0 && net2[*i_iter][k] == 0)
	    {
	      net2[*i_iter][k] = 1;
	      ++net2_edges;
	      //std::cerr << " (" << *i_iter << "," << k << ")";
	    }
	}
      //std::cerr << "\n";

      // (i)
      // Randomize the number of links to be removed.
      no_of_links = binomial(net_edges, args.lambda, generator.nextNormed());
      //std::cerr << "  (i) Removing " << no_of_links << " links (from total of " << net_edges << "):";

      // Remove the links.
      for (size_t iter = 0; iter < no_of_links; iter++)
	{
	  // Draw a random node weighed by its degree.
	  size_t i = net.weighedRandSlot(generator);
	  // Draw random link
	  size_t neighbour_no = generator.next(net(i).size());
	  size_t count = 0, j;
	  for (typename NetType::edge_iterator j_iter = net[i].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if (count == neighbour_no) 
		{
		  j = *j_iter;
		  break;
		}
	      count++;
	    }

	  //std::cerr << " (" << i << "," << j << ")";

	  //Remove link, unless if it will be added again anyway.
	  if (net2[i][j] == 0)
	    {
	      net[i][j] = 0;
	      --net_edges;
	    }
	}
      
      //std::cerr << "\n";

      // copy changes from net2 to net

      for ( size_t i = 0; i < args.netSize; ++i)
	{
	  for (typename NetType::edge_iterator j_iter = net2[i].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if ( *j_iter > i )  // take each link only once
		{
		  net[i][*j_iter] = net[i][*j_iter] + j_iter.value(); // add changes
		  j_iter.value() = 0; // remove edge so that net2 will eventually be empty
		}
	    }
	}

      // Update the number of edges
      net_edges += net2_edges;
      net2_edges = 0;

      //std::cerr << "net has " << net_edges << " (" << numberOfEdges(net) << ") edges.\n";
      //std::cerr << "net2 has " << net2_edges << " (" << numberOfEdges(net2) << ") edges.\n";

      // Just to check that the number of edges is calculated correctly.
      assert(net_edges == numberOfEdges(net));
      assert(numberOfEdges(net2) == 0);


    } // Main iteration ends
}
// <---- MarsiliCertLink



// MarsiliCertLinkRmvNode ---->
/*  
   In just like MarsiliCertLink, except that MarsiliCertLinkRmvNode
   removes random _nodes_ with probability lambda instead of random
   links.

   Note! This model should be identical to LCE2 with delta = 0.
*/
template<typename NetType, typename Generator>
void MarsiliCertLinkRmvNode(NetType& net, struct MarsiliArgs & args, Generator & generator)
{
  
  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  NetType net2(args.netSize); // initialize empty helper network

  // Keep count of the edges in the network. We start with an empty net.
  size_t net_edges = 0, net2_edges = 0;

  // If iter_max was not given as input parameter, check every
  // check_period iterations if the number of added edges (edges -
  // edges_old) is less than 1% of the number of edges now. When this
  // happens, do twice the number of iterations done by then and
  // consider it done.
  size_t check_period = 500;
  size_t edges_old = 0;
  bool end_iter = true;
  //std::cerr << "iter_max = " << args.iter_max << "\n";
  if (args.iter_max == 0)
    {
	   end_iter = false;
	   args.iter_max = 20000;
    }

  //Initialize binomial generators.
  BinomialGenerator BinGenEta(args.netSize, args.eta);
  BinomialGenerator BinGenXi(args.netSize, args.xi);
  BinomialGenerator BinGenLambda(args.netSize, args.lambda);

  for (size_t main_iter = 1; main_iter <= args.iter_max; main_iter++)
    {
      if (end_iter == false && main_iter % check_period == 0)
	{
	  if (net_edges <= edges_old || (net_edges - edges_old)/(double)net_edges < 0.01)
	    {
	      args.iter_max = 2*main_iter;
	      end_iter = true;
	      //std::cerr << "Now " << main_iter << " iterations, stopping at " << iter_max << "\n";
      	      //std::cerr << "edges = " << edges << ", edges_old = " << edges_old 
	      //          << ", prop = " << (edges - edges_old)/(double)edges << "\n";
	    }
	  
	  edges_old = net_edges;
	}

      /*
      // Print iteration number, <k> and <k²> in standard error every 100 steps.
      if (main_iter % 100 == 0)
	{
	  size_t k2 = 0;
	  for (iter = 0; iter < net.size(); ++iter) k2 += (size_t)pow(net(iter).size(), 2);

	  std::cerr << main_iter << " " << 2*(float)net_edges/(float)args.netSize << " " << (float)k2/(float)args.netSize << "\n";
	}
      */   

      //std::cerr << "Iteration " << main_iter << "\n";

      // (i)
      // Randomize the number of random links to be added.
      size_t no_of_links = BinGenEta.get(generator.nextNormed());

      //std::cerr << "  (i) Adding " << no_of_links << " random links";

      // Put links on a set to avoid duplicates.
      nodeSet random_nodes;
      while(random_nodes.size() < no_of_links) {
	random_nodes.put(generator.next(args.netSize));
      }

      // Add all empty nodes to the set.
      // (This is the only difference to Marsili2.)
      for(size_t i_iter = 0; i_iter < net.size(); ++i_iter) 
	{
	  if (net(i_iter).size() == 0) random_nodes.put(i_iter);
	}
      //std::cerr << " and " << random_nodes.size()-no_of_links << " certain links:";

      // Create random links for nodes in random_nodes.
      for(nodeSet::iterator i_iter = random_nodes.begin(); !i_iter.finished(); ++i_iter) 
	{
	  //std::cerr << " (" << *i_iter << ",";
	  size_t j;
	  do
	    {
	      j = generator.next(args.netSize);
	    } while (*i_iter == j || (net(*i_iter)[j] == 1)); // no loops of length 1, no existing links 
	  
	  //std::cerr << j << ")";

	  if (net2[*i_iter][j] == 0)
	    {
	      net2[*i_iter][j] = 1; // Add link to net2
	      ++net2_edges;
	    }
	}

      //std::cerr << "\n";

      //(ii)
      // Create a link to neighbour of a neighbour
      no_of_links = BinGenXi.get(generator.nextNormed());

      //std::cerr << "  (ii) Adding " << no_of_links << " neighbourhood links:";

      // Put links in a set to avoid duplicates
      nodeSet neighbour_nodes;
      while(neighbour_nodes.size() < no_of_links) {
	neighbour_nodes.put(generator.next(args.netSize));
      }

      // Create links to neighbours for nodes in nodeset.
      for(nodeSet::iterator i_iter = neighbour_nodes.begin(); !i_iter.finished(); ++i_iter) 
	{
	  if (net(*i_iter).size() == 0) continue; // No neighbours, continue to next random node.

	  // Select j, a random neighbour of i.
	  size_t neighbour_no = generator.next(net(*i_iter).size());
	  size_t count = 0, j;
	  for (typename NetType::edge_iterator j_iter = net[*i_iter].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if (count == neighbour_no)
		{
		  j = *j_iter;
		  break;
		}
	      count++;
	    }

	  if (net(j).size() == 1) continue; // The only neighbour of j is i, continue to next random node.

	  // Select k, a random neighbour of j, making sure k != i.
	  size_t k;
	  do {
	    size_t neighbour_no = generator.next(net(j).size());
	    size_t count = 0;
	    for (typename NetType::edge_iterator k_iter = net[j].begin(); !k_iter.finished(); ++k_iter)
	      {
		if (count == neighbour_no)
		  {
		    k = *k_iter;
		    break;
		  }
		count++;
	      }
	  } while (k == *i_iter); // no loops of length 1
	    
	  // Add a link between i and k if it does not already exist.
	  if (net[*i_iter][k] == 0 && net2[*i_iter][k] == 0)
	    {
	      net2[*i_iter][k] = 1;
	      ++net2_edges;
	      //std::cerr << " (" << *i_iter << "," << k << ")";
	    }
	}
      //std::cerr << "\n";

      // (iii)
      // Randomize the number of links to be removed.
      size_t no_of_nodes = BinGenLambda.get(generator.nextNormed());
      //std::cerr << "  (iii) Removing " << no_of_nodes << " nodes):";

      // Put links in a set to avoid duplicates
      nodeSet remove_nodes;
      while(remove_nodes.size() < no_of_nodes) {
	remove_nodes.put(generator.next(args.netSize));
      }

      // Remove the nodes.
      for(nodeSet::iterator i_iter = remove_nodes.begin(); !i_iter.finished(); ++i_iter) 
	{
	  for (typename NetType::edge_iterator j_iter = net[*i_iter].begin(); !j_iter.finished(); ++j_iter)
	    {
	      j_iter.value() = 0;
	    }

	  //std::cerr << " (" << i << "," << j << ")";
	}
      
      //std::cerr << "\n";

      // copy changes from net2 to net

      for ( size_t i = 0; i < args.netSize; ++i)
	{
	  for (typename NetType::edge_iterator j_iter = net2[i].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if ( *j_iter > i )  // take each link only once
		{
		  net[i][*j_iter] = net[i][*j_iter] + j_iter.value(); // add changes
		  j_iter.value() = 0; // remove edge so that net2 will eventually be empty
		}
	    }
	}

      // Update the number of edges
      net_edges += net2_edges;
      net2_edges = 0;

      //std::cerr << "net has " << net_edges << " (" << numberOfEdges(net) << ") edges.\n";
      //std::cerr << "net2 has " << net2_edges << " (" << numberOfEdges(net2) << ") edges.\n";

      // Just to check that the number of edges is calculated correctly.
      assert(net_edges == numberOfEdges(net));
      assert(numberOfEdges(net2) == 0);


    } // Main iteration ends
}
// <---- MarsiliCertLinkRmvNode


#endif //~ MARSILI_H

