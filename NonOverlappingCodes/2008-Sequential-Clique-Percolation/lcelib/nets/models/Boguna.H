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

// lcelib/nets/models/Boguna.H
// 
// Functions for generation of Boguna et al. networks:
//
// Marián Boguñá, romualdo Pastor-Satorras, Alvert Díaz-Guilera, Alex Arenas:
// Models of social networks based on social distance attachment
// Physical Review E 70, 056122 (2004)


#ifndef BOGUNA_H
#define BOGUNA_H
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <list>
#include "../NetExtras.H"  // required for clearNet()

const double PI(4.0 * std::atan2(1.0, 1.0));


/* The struct into which command line arguments will be read */

struct BogunaArgs 
{
  size_t netSize;
  float k_mean;
  float alpha;
  float b;
  int randseed;
};

/* readBogunaArgs --->

 N, <k> and \alpha are given as arguments. The parameter b is
 calculated so that <k> is reached.

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void readBogunaArgs(struct BogunaArgs & args, int argc, char** argv)
{
  size_t bookmark = 1;
  if ( (size_t) argc < (4+1)) {  // 4 arguments required: N, k_mean, alpha, randseed
    std::cerr << "\nPlease specify arguments: \n\tN, k_mean, alpha, randseed\n\n"; exit(1);
  }  
  args.netSize = atoi(argv[bookmark]);  bookmark++;
  args.k_mean = atof(argv[bookmark]); bookmark++;
  args.alpha = atof(argv[bookmark]); bookmark++;
  args.randseed = atoi(argv[bookmark]);  bookmark++;

  // Check validity of input parameters.

  if ( args.alpha < 1 )
    {
      std::cerr << "\nThe value of alpha (now " << args.alpha << ") must be greater than 1.\n";
      exit(1);
    }

  if ( args.k_mean <= 0 )
    {
      std::cerr << "\nThe value of <k> (now " << args.k_mean << ") must be greater than 0.\n";
      exit(1);
    }      

  args.b = args.alpha*sin(PI/args.alpha)*args.k_mean/(2*args.netSize*PI);

}  
// <--- readBogunaArgs


// outputBogunaArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void outputBogunaArgs(struct BogunaArgs & args) 
{
  std::cerr << "Parameters given for simulating Boguna network:\n";
  std::cerr << "(arguments required: N, k_mean, alpha, randseed)\n";
  std::cerr << "Network size:\t\t" << args.netSize << "\n"; 
  std::cerr << "k_mean:\t\t\t" << (float) args.k_mean << "\n";
  std::cerr << "alpha:\t\t\t" << (float) args.alpha << "\n";
  std::cerr << "b:\t\t\t" << (float) args.b << "\n\n";

  std::cerr << "Random number generator seed (integer): " << args.randseed << "\n";
  std::cerr << "\n";
} 
// <--- outputBogunaArgs
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


// Boguna ---->
/*  
 Generates an unweighted Boguna network.

*/

// A small class that contains a 1-d point with ID.
class IDpoint
{
public:
  IDpoint(size_t id, double x):id(id),x(x) {};

  size_t id;
  double x;
};

bool IDpoint_sort_function(IDpoint* p1, IDpoint* p2) { return (p1->x < p2->x); };

template<typename NetType, typename Generator>
void Boguna(NetType& net, struct BogunaArgs & args, Generator & generator)
{

  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  std::list<IDpoint*> points;     // For N random points.

  // Randomize N points in interval [0,1]. Take care of periodic borders.
  for (size_t i = 0; i < args.netSize; i++)
      points.push_back(new IDpoint(i, generator.nextNormed()));

  // Sort the points in ascending order according to x.
  points.sort(IDpoint_sort_function);

  // Go through all edges
  for (std::list<IDpoint*>::iterator i_iter = points.begin(); i_iter != --points.end(); ++i_iter)
    {
      std::list<IDpoint*>::iterator j_iter = i_iter;
      ++j_iter;

      //std::cerr << "Point " << (*i_iter)->id << " is at (" << (*i_iter)->x << ", " << (*i_iter)->y << ")\n"; 

      for (; j_iter != points.end() ; ++j_iter)
	{
	  // Calculate probability of making a link. Because the list
	  // is sorted, (*j_iter)->x is always larger than
	  // (*i_iter)->x.
	  double r = 1/(1+pow( ((*j_iter)->x - (*i_iter)->x)/args.b, args.alpha));

	  // Create link with probability r.
	  if (r > generator.nextNormed())
	    {
	      net[(*i_iter)->id][(*j_iter)->id] = 1;
	    }
	}
    }

  // Delete all IDpoints.
  for (std::list<IDpoint*>::iterator i_iter = points.begin(); i_iter != points.end(); i_iter++)
    {
      delete *i_iter;
    }

}


#endif //~ BOGUNA_H

