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

// lcelib/nets/models/Davidsen.H
// 
// Functions for generation of Davidsen networks
//
// (Jörn Davidsen, Holger Ebel, Stefan Bornholdt: 
// Emergence of a Small World from Local Interactions: Modeling Acquaintance Networks,
// Physical Review Letters, vol 88, number 12, 25 March 2002)

/*
 Function list
 ­------------

Davidsen:
Generate a network according to the Davidsen model.

DavidsenRmvLink:
The algorith is exactly as in Davidsen, expect that random links are
removed instead of nodes. With probability p we select a random node i
and remove k(i)-1 random links from the network.

DavidsenStep:
The algorith is exactly as in Davidsen, expect that instead of linking
two neighbors we make a 2-step random walk and link the final node to
the one the walk started from.

DavidsenStepRmvLink:
The algorith is exactly as in DavidsenStep, expect that node removal
is carried out as in DavidsenRmvLink.

*/


/*
 Usage
 -----

 ./Davidsen 1000 0.05 3468543 > network.edg

 Generates a Davidsen network with N = 1000, p = 0.05 and random seed
 3468543.  N is the number of nodes and p is the probability for
 removing one random node during each iteration.

 p, mean degree <k> and clustering coefficient C are connected by equation
    1 - C = p (<k> - 1)   .
 Because C is approximately 0.4 for a vast number of parameters, parameter p
 may be approximated by 
    p = 0.6 / (<k> - 1)   .
 For instance, N = 10000 and p = 0.0822 produces a network with <k> = 8.

*/


#ifndef DAVIDSEN_H
#define DAVIDSEN_H
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include "../NetExtras.H"  // required for clearNet()


/* The struct into which command line arguments will be read */

struct DavidsenArgs 
{
  size_t netSize;
  float p; 
  int randseed;
  size_t iter_max;
};

// readDavidsenArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void readDavidsenArgs(struct DavidsenArgs & args, int argc, char** argv)
{
  size_t bookmark = 1;
  if ( (size_t) argc < (3+1)) {  // 3 arguments required: N, p, randseed
    std::cerr << "\nPlease specify arguments: \n\tN, p, randseed\n\n"; exit(1);
  }  
  args.netSize = atoi(argv[bookmark]);  bookmark++;
  args.p = atof(argv[bookmark]); bookmark++;
  args.randseed = atoi(argv[bookmark]);  bookmark++;
  
  /* The number of iterations is chosen be setting the expected number
     of nodes that are never removed during the iteration (N_nr, by
     default 0.5). */
  float N_nr = 0.5;
  args.iter_max = static_cast<size_t>( (log(N_nr) - log(args.netSize)) / log(1-args.p/args.netSize));
}  
// <--- readDavidsenArgs


// outputDavidsenArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void outputDavidsenArgs(struct DavidsenArgs & args) 
{
  std::cerr << "Parameters given for simulating Davidsen network:\n";
  std::cerr << "(arguments required: N, p, randseed)\n\n";
  std::cerr << "Network size:\t\t\t\t\t" << args.netSize << "\n"; 
  std::cerr << "p (probability of removing a node):\t\t" << (float) args.p << "\n";

  std::cerr << "Number of iterations: \t\t\t\t" << args.iter_max << "\n";
  std::cerr << "Random number generator seed (integer): \t" << args.randseed << "\n";
  std::cerr << "\n\n";
} 
// <--- outputDavidsenArgs
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


// Davidsen ---->
/*  
 Generates an unweighted Davidsen network. The network is initially an
 empty, fixed size network and is constructed by iterating the
 following two rules:
 
 (i) Pick a random node. If the node has two or more neighbours,
 randomly pick two neighbours and add a link between them. If the
 random node has one or zero neighbours, add a link to another
 randomly selected node. 

 (ii) With probability p, remove one randomly selected node (and all
 of its links) from the network. After the node is removed, add a new
 node with one link to a randomly selected node.

*/


template<typename NetType, typename Generator>
void Davidsen(NetType& net, struct DavidsenArgs & args, Generator & generator)
{

  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  size_t i, j, k;

  for (size_t iter = 1; iter <= args.iter_max; iter++)
    {
      // (i)
      // Select random node
      i = generator.next(args.netSize);
      if (net(i).size() < 2)
	{
	  // 0 or 1 neighbour ==> add link to a random node
	  do
	    {
	      j = generator.next(args.netSize);
	      assert(j < args.netSize);
	    }
	  while ( (i == j) || net(i)[j] > 0 ); /* no loops of length 1, no multiple edges */
	  
	  net[i][j] = 1; /* add edge between nodes i and j */
	}
      else
	{
	  /* Two or more neighbours ==> add link between two random neighbours. */
	  do
	    {
	      j = generator.next(net(i).size());
	      k = generator.next(net(i).size());
	    }
	  while ( (j == k) ); /* no loops of length 1 */
	  
	  size_t node_1 = 0, node_2 = 0;
	  size_t count = 0;
	  for (typename NetType::edge_iterator j_iter = net[i].begin();
	       !j_iter.finished();
	       ++j_iter)
	    {
	      if (count == (j<k?j:k)) node_1 = *j_iter;
	      if (count == (j>k?j:k)) node_2 = *j_iter;
	      count++;
	    }
	  
	  net[node_1][node_2] = 1; // Add link between the two neighbours.

	}

      //(ii) Delete one random node with probability p, and add a new node. 
      
      if (generator.nextNormed() < args.p)
	{
	  // Remove one random node
	  i = generator.next(args.netSize);

	  for (typename NetType::edge_iterator j_iter = net[i].begin();
	       !j_iter.finished();
	       ++j_iter)
	    {
	      j_iter.value() = 0;
	    }

	  // Add a new node and link it to one random node.
	  do
	    {
	      j = generator.next(args.netSize);
	      assert(j < args.netSize);
	    }
	  while (i == j); /* no loops of length 1 */

	  net[i][j] = 1;
	}

    } // end main loop

}
// <---- Davidsen



// DavidsenRmvLink ---->
/*  
 Generates an unweighted Davidsen network with a small twist: instead
 of removing a node with probability p, we remove an equal amount of
 random links. The network is initially an empty, fixed size network
 and is constructed by iterating the following two rules:
 
 (i) Pick a random node. If the node has two or more neighbours,
 randomly pick two neighbours and add a link between them. If the
 random node has one or zero neighbours, add a link to another
 randomly selected node. 

 (ii) With probability p, select a random node i:
      - if k(i) = 0, link i to a random node.
      - if k(i) = 1, do nothing
      - if k(i) > 1, remove k(i)-1 random links from the network.

*/

template<typename NetType, typename Generator>
void DavidsenRmvLink(NetType& net, struct DavidsenArgs & args, Generator & generator)
{

  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  size_t i, j, k;

  for (size_t iter = 1; iter <= args.iter_max; iter++)
    {

      // (i)
      // Select random node
      i = generator.next(args.netSize);
      if (net(i).size() < 2)
	{
	  // 0 or 1 neighbour ==> add link to a random node
	  do
	    {
	      j = generator.next(args.netSize);
	      assert(j < args.netSize);
	    }
	  while ( (i == j) || net(i)[j] > 0 ); /* no loops of length 1, no multiple edges */
	  
	  net[i][j] = 1; /* add edge between nodes i and j */
	}
      else
	{
	  /* Two or more neighbours ==> add link between two random neighbours. */
	  do
	    {
	      j = generator.next(net(i).size());
	      k = generator.next(net(i).size());
	    }
	  while ( (j == k) ); /* no loops of length 1 */
	  
	  size_t node_1 = 0, node_2 = 0;
	  size_t count = 0;
	  for (typename NetType::edge_iterator j_iter = net[i].begin();
	       !j_iter.finished();
	       ++j_iter)
	    {
	      if (count == (j<k?j:k)) node_1 = *j_iter;
	      if (count == (j>k?j:k)) node_2 = *j_iter;
	      count++;
	    }
	  
	  net[node_1][node_2] = 1; // Add link between the two neighbours.

	}

      //(ii) Remove links.
      if (generator.nextNormed() < args.p)
	{
	  // Sample a node to find out how many links to remove.
	  i = generator.next(args.netSize);
	  size_t no_of_links =  net(i).size();

	  if (no_of_links == 0)
	    {
	      // The original Davidsen algorithm effectively adds a
	      // random link if an empty node is selected for
	      // removal. To be consistent, we have to do that also.
	      do
		{
		  j = generator.next(args.netSize);
		  assert(j < args.netSize);
		}
	      while ( (i == j) || net(i)[j] > 0 ); /* no loops of length 1, no multiple edges */
	      net[i][j] = 1; /* add edge between nodes i and j */
	    }
	  else if (no_of_links == 1)
	    {
	      // The original Davidsen algorithm rewires a link if a
	      // node with only one link is selected for removal. This
	      // should have no effect on the algorithm (or does it?),
	      // so we skip the rewiring.
	    }
	  else
	    {
	      // The original Davidsen algorithm would add one random
	      // link after removing all links of node i. Therefore we
	      // must remove only net(i).size()-1 links from the net.
	      --no_of_links; 

	      // Remove the links.
	      size_t neighbour_no, count;
	      for (size_t link_iter = 0; link_iter < no_of_links; link_iter++)
		{
		  // Draw a random node weighed by its degree.
		  j = net.weighedRandSlot(generator);
		  // Draw a random link
		  neighbour_no = generator.next(net(j).size());
		  count = 0;
		  k = 0;
		  for (typename NetType::edge_iterator k_iter = net[j].begin(); !k_iter.finished(); ++k_iter)
		    {
		      if (count == neighbour_no) 
			{
			  k = *k_iter;
			  break;
			}
		      count++;
		    }

		  //Remove link
		  net[j][k] = 0;
		}
	    }
	} // end (ii) Remove links.
    } // end main loop
}
// <---- DavidsenRmvLink


// DavidsenStep ---->
/*  
 Generates an unweighted Davidsen network, except that the linking of
 neighbours is carried out by a random walk. The network is initially
 an empty, fixed size network and is constructed by iterating the
 following two rules:

 (i) Pick a random node. If the node has two or more neighbors,
 randomly pick a neighbor's neighbor and add a link between that node
 and the original node. If the random node has one or zero neighbors,
 add a link to another randomly selected node.
 
 (ii) With probability p, remove one randomly selected node (and all
 of its links) from the network. After the node is removed, add a new
 node with one link to a randomly selected node.

*/


template<typename NetType, typename Generator>
void DavidsenStep(NetType& net, struct DavidsenArgs & args, Generator & generator)
{

  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  size_t i, j = 0, k = 0, neighbor_no, count;

  for (size_t iter = 1; iter <= args.iter_max; iter++)
    {

      /*
      // Output something every 1000 iterations
      if (iter % 1000 == 0)
	{
	  std::cerr << "iter = " << iter << "/" << args.iter_max << "\n";
	}
      */

      // (i)
      // Select random node
      i = generator.next(args.netSize);
      //std::cerr << iter << ": selected node " << i << "\n";
      if (net(i).size() < 2)
	{
	  // one or zero neighbors ==> add link to a random node
	  do
	    {
	      j = generator.next(args.netSize);
	      assert(j < args.netSize);
	    }
	  while (i == j || net(i)[j] > 0); /* no loops of length 1 */
	  
	  net[i][j] = 1; /* add edge between nodes i and j */
	  //std::cerr << "   ...added random link to node " << j << "\n";
	}
      else
	{
	  /* At least two neighbors */

	  // Select j, a random neighbour of i.
	  neighbor_no = generator.next(net(i).size());
	  //std::cerr << "   ...has " << net(i).size() << " neighbors\n";
	  count = 0;
	  for (typename NetType::edge_iterator j_iter = net[i].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if (count == neighbor_no)
		{
		  j = *j_iter;
		  break;
		}
	      count++;
	    }

	  //std::cerr << "   ...selected neighbor " << j << ", which has " << net(j).size() << " neighbors\n";

	  if (net(j).size() > 1)
	    {
	      // Select k, a random neighbour of j, making sure k != i.
	      do {
		neighbor_no = generator.next(net(j).size());
		count = 0;
		for (typename NetType::edge_iterator k_iter = net[j].begin(); !k_iter.finished(); ++k_iter)
		  {
		    if (count == neighbor_no)
		      {
			k = *k_iter;
			break;
		      }
		    count++;
		  }
	      } while (k == i); // no loops of length 1
	    
	      // Add a link between i and k if it does not already exist.
	      net[i][k] = 1;
	      //std::cerr << "   ...added neighbor link from node " << i << " to node " << k << "\n";
	    }
	} // end (i)

      
      //(ii) Delete one random node with probability p, and add a new node. 
      
      if (generator.nextNormed() < args.p)
	{
	  // Remove one random node
	  i = generator.next(args.netSize);
	  
	  for (typename NetType::edge_iterator j_iter = net[i].begin();
	       !j_iter.finished();
	       ++j_iter)
	    {
	      j_iter.value() = 0;
	    }

	  // Add a new node and link it to one random node.
	  do
	    {
	      j = generator.next(args.netSize);
	      assert(j < args.netSize);
	    }
	  while (i == j); /* no loops of length 1 */

	  net[i][j] = 1;
	}

    } // end main loop

}
// <---- DavidsenStep



// DavidsenStepRmvLink ---->
/*  
 Generates an unweighted DavidsenStep network with a small twist:
 instead of removing a node with probability p, we remove an equal
 amount of random links. The network is initially an empty, fixed size
 network and is constructed by iterating the following two rules:
 
 (i) Pick a random node. If the node has two or more neighbors,
 randomly pick a neighbor's neighbor and add a link between that node
 and the original node. If the random node has one or zero neighbors,
 add a link to another randomly selected node.
 
 (ii) With probability p, select a random node i:
      - if k(i) = 0, link i to a random node.
      - if k(i) = 1, do nothing
      - if k(i) > 1, remove k(i)-1 random links from the network.

*/

template<typename NetType, typename Generator>
void DavidsenStepRmvLink(NetType& net, struct DavidsenArgs & args, Generator & generator)
{

  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  size_t i, j, k;

  for (size_t iter = 1; iter <= args.iter_max; iter++)
    {

      // (i)
      // Select random node
      i = generator.next(args.netSize);
      //std::cerr << iter << ": selected node " << i << "\n";
      if (net(i).size() < 2)
	{
	  // one or zero neighbors ==> add link to a random node
	  do
	    {
	      j = generator.next(args.netSize);
	      assert(j < args.netSize);
	    }
	  while (i == j || net(i)[j] > 0); /* no loops of length 1 */
	  
	  net[i][j] = 1; /* add edge between nodes i and j */
	  //std::cerr << "   ...added random link to node " << j << "\n";
	}
      else
	{
	  /* At least two neighbors */

	  // Select j, a random neighbour of i.
	  size_t neighbor_no = generator.next(net(i).size());
	  //std::cerr << "   ...has " << net(i).size() << " neighbors\n";
	  size_t count = 0;
	  for (typename NetType::edge_iterator j_iter = net[i].begin(); !j_iter.finished(); ++j_iter)
	    {
	      if (count == neighbor_no)
		{
		  j = *j_iter;
		  break;
		}
	      count++;
	    }

	  //std::cerr << "   ...selected neighbor " << j << ", which has " << net(j).size() << " neighbors\n";

	  if (net(j).size() > 1)
	    {
	      // Select k, a random neighbour of j, making sure k != i.
	      do {
		neighbor_no = generator.next(net(j).size());
		count = 0; k = 0;
		for (typename NetType::edge_iterator k_iter = net[j].begin(); !k_iter.finished(); ++k_iter)
		  {
		    if (count == neighbor_no)
		      {
			k = *k_iter;
			break;
		      }
		    count++;
		  }
	      } while (k == i); // no loops of length 1
	    
	      // Add a link between i and k if it does not already exist.
	      net[i][k] = 1;
	      //std::cerr << "   ...added neighbor link from node " << i << " to node " << k << "\n";
	    }
	} // end (i)


      //(ii) Remove links.
      if (generator.nextNormed() < args.p)
	{
	  // Sample a node to find out how many links to remove.
	  i = generator.next(args.netSize);
	  size_t no_of_links =  net(i).size();

	  if (no_of_links == 0)
	    {
	      // The original Davidsen algorithm effectively adds a
	      // random link if an empty node is selected for
	      // removal. To be consistent, we have to do that also.
	      do
		{
		  j = generator.next(args.netSize);
		  assert(j < args.netSize);
		}
	      while ( (i == j) || net(i)[j] > 0 ); /* no loops of length 1, no multiple edges */
	      net[i][j] = 1; /* add edge between nodes i and j */
	    }
	  else if (no_of_links == 1)
	    {
	      // The original Davidsen algorithm rewires a link if a
	      // node with only one link is selected for removal. This
	      // should have no effect on the algorithm (or does it?),
	      // so we skip the rewiring.
	    }
	  else
	    {
	      // The original Davidsen algorithm would add one random
	      // link after removing all links of node i. Therefore we
	      // must remove only net(i).size()-1 links from the net.
	      --no_of_links; 

	      // Remove the links.
	      size_t neighbour_no, count;
	      for (size_t link_iter = 0; link_iter < no_of_links; link_iter++)
		{
		  // Draw a random node weighed by its degree.
		  j = net.weighedRandSlot(generator);
		  // Draw a random link
		  neighbour_no = generator.next(net(j).size());
		  count = 0;
		  k = 0;
		  for (typename NetType::edge_iterator k_iter = net[j].begin(); !k_iter.finished(); ++k_iter)
		    {
		      if (count == neighbour_no) 
			{
			  k = *k_iter;
			  break;
			}
		      count++;
		    }

		  //Remove link
		  net[j][k] = 0;
		}
	    }
	} // end (ii) Remove links.
    } // end main loop
}
// <---- DavidsenStepRmvLink


#endif //~ DAVIDSEN_H

