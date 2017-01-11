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

// lcelib/nets/models/Wong.H
// 
// Functions for generation of Wong et al. networks:
//
// Ling Heng Wong, Pholippa Pattison, Garry Robins
// A spatial model for social networks:
// Modelling Acquaintance Networks,
// Physica A 360 (2006) 99-120)



#ifndef WONG_H
#define WONG_H
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>
#include "../NetExtras.H"  // required for clearNet()

const double PI(4.0 * std::atan2(1.0, 1.0));


/* The struct into which command line arguments will be read */

struct WongArgs 
{
  size_t netSize;
  float k_mean;
  float H;
  float p_b;
  float p;
  float d;
  int randseed;
};

/* readWongArgs --->

 N, <k>, H and p_b are given as arguments. The probability p =
 <k>/(N-1) and d is calculated so that the wanted <k> is reached.

 Note that there is an upper limit for p_b, which depends on radius
 H. The limit is caused by the fact that (p-d) must be larger than
 zero. If the given p_b is larger than the upper limit, an error
 warning stating the upper limit is printed and the program exits.

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void readWongArgs(struct WongArgs & args, int argc, char** argv)
{
  size_t bookmark = 1;
  if ( (size_t) argc < (5+1)) {  // 5 arguments required: N, k_mean, H, p_b, randseed
    std::cerr << "\nPlease specify arguments: \n\tN, k_mean, H, p_b, randseed\n\n"; exit(1);
  }  
  args.netSize = atoi(argv[bookmark]);  bookmark++;
  args.k_mean = atof(argv[bookmark]); bookmark++;
  args.H = atof(argv[bookmark]); bookmark++;
  args.p_b = atof(argv[bookmark]); bookmark++;
  args.randseed = atoi(argv[bookmark]);  bookmark++;

  args.p = args.k_mean/(args.netSize - 1);

  // Check validity of input parameters.

  if ( args.H < 0 )
    {
      std::cerr << "\nThe value of H (now " << args.H << ") must be greater than 0.\n";
      exit(1);
    }      

  if ( args.p_b < 0 )
    {
      std::cerr << "\nThe value of p_b (now " << args.p_b << ") must be greater than 0.\n";
      exit(1);
    }      

  // The upper bound for H is sqrt(1/(PI)) = 0.5642,
  // the value when the area of circle with radius H is 1.
  double H_max =  1 / sqrt(PI);
  if ( args.H > H_max )
    {
      std::cerr << "\nThe value of H (now " << args.H << ") must be less than " << H_max << "\n";
      exit(1);
    }      

  // p_b must be less than min( 1-p , (1/(PI*H^2) - 1)*p )
  // The first limit comes from condition p+p_b <= 1, the second one
  // from p-d >= 0.
  double p_b_max = (1/(PI*pow(args.H, 2)) - 1)*args.p;
  p_b_max = (1-args.p < p_b_max ? 1-args.p : p_b_max);
  if (args.p_b > p_b_max)
    {
      std::cerr << "\nThe value of p_b (now " << args.p_b << ") must be less than " << p_b_max << "\n";
      exit(1);
    }
  
  // Calculate d
  args.d = args.p_b*(PI*pow(args.H, 2))/(1-PI*pow(args.H, 2));

}  
// <--- readWongArgs


// outputWongArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void outputWongArgs(struct WongArgs & args) 
{
  std::cerr << "Parameters given for simulating Wong network:\n";
  std::cerr << "(arguments required: N, k_mean, H, p_b, randseed)\n";
  std::cerr << "Network size:\t\t" << args.netSize << "\n"; 
  std::cerr << "k_mean:\t\t\t" << (float) args.k_mean << "\n";
  std::cerr << "radius H:\t\t" << (float) args.H << "\n";
  std::cerr << "p_b:\t\t\t" << (float) args.p_b << "\n\n";

  std::cerr << "Calculated parameters:\n";
  std::cerr << "p:\t\t\t" << (float) args.p << "\n";
  std::cerr << "d:\t\t\t" << (float) args.d << "\n\n";

  std::cerr << "Random number generator seed (integer): " << args.randseed << "\n";
  std::cerr << "\n";
} 
// <--- outputWongArgs
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


// Wong ---->
/*  
 Generates an unweighted Wong network.

 First, N 2-d points are randomized on a unit square. Then an edge is
 added with probability (p+p_b) if the distance between the two points
 is less than H and with probability (p-d) otherwise.

 Periodic borders are used so that each node has an equal connection
 potential, that is, each node has the same expected number of nodes
 inside radius H even if the node is located at the very edge of the
 unit square.


*/

// A small class that contains a 2-d point with ID.
class IDpoint 
{
public:
  IDpoint(size_t id, double x, double y):id(id),x(x),y(y) {};

  size_t id;
  double x;
  double y;
};


template<typename NetType, typename Generator>
void Wong(NetType& net, struct WongArgs & args, Generator & generator)
{

  ClearNet(net, args.netSize); /* make sure there are no edges present to start with */

  size_t i;
  bool left_margin = false, right_margin = false;
  bool high_margin = false, low_margin = false;
  std::vector<IDpoint*> points;     // For N random 2-dimensional points.
  std::vector<IDpoint*> points_ext; // The same as points, but with added points because of periodic border.

  // Randomize N points in unit square. Take care of periodic borders.
  for (i = 0; i < args.netSize; i++)
    {
      IDpoint* pnt = new IDpoint(i, generator.nextNormed(), generator.nextNormed());
      points.push_back(pnt);
      points_ext.push_back(pnt);

      // Check if the point is closer than H to any margin.  A maximum
      // of 2 statements may be true, if the point lies in the corner
      // of the unit square.
      if (pnt->x < args.H) left_margin = true;
      if (pnt->x > 1-args.H) right_margin = true;
      if (pnt->y < args.H) low_margin = true;
      if (pnt->y > 1-args.H) high_margin = true;

      // Copy the point if it is located at left, right, lower and
      // upper margin.
      if (left_margin)
	points_ext.push_back(new IDpoint(i, pnt->x + 1, pnt->y));
      else if (right_margin)
	points_ext.push_back(new IDpoint(i, pnt->x - 1, pnt->y));
      if (low_margin)
	points_ext.push_back(new IDpoint(i, pnt->x, pnt->y + 1));
      else if (high_margin)
	points_ext.push_back(new IDpoint(i, pnt->x, pnt->y - 1));

      // Copy the point if it is located at any of the corners.
      // upper margin.
      if (left_margin && low_margin)
	points_ext.push_back(new IDpoint(i, pnt->x + 1, pnt->y + 1));
      else if (low_margin && right_margin)
	points_ext.push_back(new IDpoint(i, pnt->x - 1, pnt->y + 1));
      else if (right_margin && high_margin)
	points_ext.push_back(new IDpoint(i, pnt->x - 1, pnt->y - 1));
      else if (high_margin && left_margin)
	points_ext.push_back(new IDpoint(i, pnt->x + 1, pnt->y - 1));

      left_margin = false;
      right_margin = false;
      high_margin = false;
      low_margin = false;
    }

  //std::cerr << "Random points succesfully generated.\n";
  //std::cerr << "Size of points:     " << points.size() << "\n";
  //std::cerr << "Size of points_ext: " << points_ext.size() << "\n\n";

  std::vector<IDpoint*>::iterator i_iter, j_iter, j_iter_start;
  bool H_test = false; // To test if distance between points is less than H.

  // Initialize j_iter at second node
  j_iter_start = points_ext.begin();
  while ((*j_iter_start)->id == 0) j_iter_start++;

  // Go through all edges
  for (i_iter = points.begin(); i_iter != --points.end(); i_iter++)
    {
      //std::cerr << "Point " << (*i_iter)->id << " is at (" << (*i_iter)->x << ", " << (*i_iter)->y << ")\n"; 

      for (j_iter = j_iter_start; j_iter != points_ext.end() ; )
	{
	  //std::cerr << "  Point " << (*j_iter)->id << " is at (" << (*j_iter)->x << ", " << (*j_iter)->y << "), copied "; 
	  
	  // Test if |x(i)-x(j)| < H
	  // Go through all copies of point j made due to periodic porders.
	  H_test = false;
	  size_t ID = (*j_iter)->id;
	  do
	    {
	      H_test = H_test || (fabs((*i_iter)->x - (*j_iter)->x) <= args.H &&
				  fabs((*i_iter)->y - (*j_iter)->y) <= args.H &&
				  sqrt(pow((*i_iter)->x - (*j_iter)->x, 2) + pow((*i_iter)->y - (*j_iter)->y, 2)) <= args.H );
	      j_iter++;
	      //std::cerr << "x";
	    } while(j_iter != points_ext.end() && (*j_iter)->id == ID);

	  //std::cerr << ", H_test = " << H_test << "\n"; 

	  // Add edge
	  if (H_test) 
	    {
	      // Inside radius H
	      if (generator.nextNormed() < args.p + args.p_b) net[(*i_iter)->id][ID] = 1;
	    }
	  else
	    {
	      // Outside radius H
	      if (generator.nextNormed() < args.p - args.d) net[(*i_iter)->id][ID] = 1;
	    }

	}

      // Move starting index to next node
      while (j_iter_start != points_ext.end() && (*j_iter_start)->id == (*i_iter)->id + 1) j_iter_start++;
    }
 
  // Delete all IDpoints. We only need to go through points_ext
  // because every IDpoint in points is also included in points_ext.
  for (i_iter = points_ext.begin(); i_iter != points_ext.end(); i_iter++)
    {
      delete *i_iter;
    }

}


#endif //~ WONG_H

