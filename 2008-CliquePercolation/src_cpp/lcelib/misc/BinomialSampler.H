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

// lcelib/misc/BinomialSampler.H
// Code for sampling numbers from binomial distribution.
//
// Author: Lauri Kovanen, 25 July 2007

/* 
   Usage
   -----
   

   class BinomialGenerator
   
   Use this class if you need to generate several numbers from the
   same binomial distribution. Create the generator with constructor
     BinomialGenerator(size_t N, double p)
   where N and p are the parameters of the distribution Bin(N,p). To
   get a sample from the distribution, call method
     size_t get(double rand_num)
   with rand_num being a number in interval [0,1]. To generate random
   samples, you need to have a random number generator and then call
   for instance
     BinomialGenerator BinGen(10, 0.2);
     size_t sample = BinGen.get(generator.nextNormed());


   function binomial

   If you only need to generate a single sample from a distribution
   there is no need to create a class instance. Instead, use this
   simple function instead. The function call is
     size_t binomial(size_t N, float p, double rand_normed)
   For example, to generate a single sample from Bin(10, 0.2), use
     size_t sample = binomial(10, 0.2, generator.nextNormed());


*/

#ifndef BINSAMPLER_H
#define BINSAMPLER_H

#include <iostream>
#include <cmath>
#include <list>

/* class BinomialGenerator

Class for creating several samples from the same binomial distribution.

*/
class BinomialGenerator
{
public:
  
  // Standard constructor that makes no sense.
  BinomialGenerator() { p = 0.5; N = 2; exp_val = 1; }
  
  // The constructor the user should really use.
  BinomialGenerator(size_t _N, double _p) : N(_N),p(_p)
  {
    if (p <= 0 || p >= 1)
      {
	std::cerr << "Parameter p must be in (0,1).\n";
	exit(0);
      }

    exp_val = (size_t)floor((double)N*p);
    if (p <= 0.5) init_low_list();
    else init_high_list();

    /*
    std::cout << "high_list:\n";
    for (std::list<double>::iterator iter = high_list.begin(); iter != high_list.end(); ++iter)
	std::cout << *iter << "\n";
    std::cout << "\n\n";

    std::cout << "low_list:\n";
    for (std::list<double>::iterator iter = low_list.begin(); iter != low_list.end(); ++iter)
	std::cout << *iter << "\n";
    std::cout << "\n\n";
    */
  }

  // Call next for a random number from binomial distribution with
  // parameters (N,p).
  size_t get(double rand_num)
  {

    size_t res = exp_val + 1;

    std::list<double>::iterator iter = low_list.end();
    if (rand_num < *(--iter) )
      {
	//Go through the low_list until the correct value is found.
	while (iter != low_list.begin())
	  {
	    if (rand_num >= *iter) return res;
	    --res;
	    --iter;
	  }
	// Correct value was not found - need to expand low_list.
	return expand_low_list(rand_num);
      }
    else
      {
	//Go through the high_list until the correct value is found.
	iter = high_list.begin();
	while (iter != high_list.end())
	  {
	    if (rand_num <= *iter) return res;
	    ++res;
	    ++iter;
	  }
	// Correct value was not found - need to expand high_list.
	return expand_high_list(rand_num);
      }

    return res; // This should never happen.
  };

private:

  void init_low_list()
  {
    double prob_sum = 0;
    size_t counter = 0;
    double log_bincoeff = 0;       // equals log(N over 0) initially, will be updated
    for (size_t i = 0; i <= exp_val; ++i)
      {
	prob_sum += exp(log_bincoeff + counter*log(p) + (N - counter)*log(1-p));
	// Update binomial coefficient for next round.
	log_bincoeff += log((double)(N - counter));
	counter++;
	log_bincoeff -= log((double)counter);
      }

    low_list.push_back(prob_sum);
    prob_sum += exp(log_bincoeff + counter*log(p) + (N - counter)*log(1-p));
    high_list.push_back(prob_sum);
    return;
  };

  void init_high_list()
  {
    double prob_sum = 1;
    size_t counter = N;
    double log_bincoeff = 0;       // equals log(N over N) initially, will be updated
    for (size_t i = N; i > exp_val + 1; --i)
      {
	prob_sum -= exp(log_bincoeff + counter*log(p) + (N - counter)*log(1-p));
	// Update binomial coefficient for next round.
	log_bincoeff += log((double)counter);
	--counter;
	log_bincoeff -= log((double)(N - counter));
      }

    high_list.push_front(prob_sum);
    prob_sum -= exp(log_bincoeff + counter*log(p) + (N - counter)*log(1-p));
    low_list.push_back(prob_sum);
    return;
  };

  // Expand low list until value rand is reached.
  size_t expand_low_list(double rand)
  {
    double prob_sum(low_list.front());
    size_t counter = exp_val - low_list.size() + 1;
    // equals log(N over counter) initially, will be updated
    double log_bincoeff = 0;
    for (size_t n = N; n >= N - counter + 1; --n) log_bincoeff += log(n);
    for (size_t n = 1; n <= counter; ++n) log_bincoeff -= log(n);

    while (rand < prob_sum)
      {
	prob_sum -= exp(log_bincoeff + counter*log(p) + (N - counter)*log(1-p));
	low_list.push_front(prob_sum);

	// Update binomial coefficient for next round.
	if (counter > 0)
	  {
	    log_bincoeff += log((double)counter);
	    --counter;
	    log_bincoeff -= log((double)(N - counter));
	  }
	else
	  {
	    low_list.push_front(0);
	    return 0;
	  }
      }

    return counter + 1;
  };

  // Expand high_list until value rand is reached.
  size_t expand_high_list(double rand)
  {
    double prob_sum(high_list.back());
    size_t counter = exp_val + high_list.size() + 1;
    // equals log(N over counter) initially, will be updated
    double log_bincoeff = 0;
    for (size_t n = N; n >= N - counter + 1; --n) log_bincoeff += log(n);
    for (size_t n = 1; n <= counter; ++n) log_bincoeff -= log(n);

    while (rand > prob_sum)
      {
	prob_sum += exp(log_bincoeff + counter*log(p) + (N - counter)*log(1-p));
	high_list.push_back(prob_sum);

	// Update binomial coefficient for next round.
	if (counter < N)
	  {
	    log_bincoeff += log((double)(N - counter));
	    counter++;
	    log_bincoeff -= log((double)counter);
	  }
	else {
	  high_list.push_back(1);
	  return N;
	}
      }

    return counter - 1;
  };

  size_t N;
  double p;
  size_t exp_val; // Expected value

  std::list<double> high_list;
  std::list<double> low_list;

}; // end class Binomial


/* binomial

   Returns a value sampled from a Binomial distribution. Works
   reasonably fast if probability p is small. If you are going to
   randomize several samples from the same binomial distribution, use
   class BinomialGenerator instead.

   There is a peculiar problem if rand_normed is either 0 or
   1. Technically, the correct return value would be 0 for rand_normed
   = 0 and N for rand_normed = 1. However, in most cases the
   probability of 0 or N would be too high. Consider for instance
   distribution Bin(1000, 0.01) where the probability for N would be
   1e-10000. However, if rand_normed is obtained from, for instance,
   Mersenne Twister with interval [0,1], the probability of getting 1
   is roughly 4.2e-9.

   While an event with probability 4.2e-9 could still happen, one
   would have to wait until the end of the world to see an event with
   probability 1e-10000.
 */
size_t binomial(size_t N, float p, double rand_normed)
{
  //std::cerr << "    Calculating binomial for (N,p) = (" << N << ", " << p << "), random number = " << rand_normed << "\n";
  if (rand_normed == 0) rand_normed = 1e-10;
  if (rand_normed == 1) rand_normed = 1 - 1e-10;
  double prob_sum = 0;
  size_t counter = 0;
  double log_prod = 0;       // equals log(N over 0) initially, will be updated
  while (rand_normed > prob_sum)
  {
    prob_sum += exp(log_prod + counter*log(p) + (N - counter)*log(1-p));
    log_prod += log((double)(N - counter));
    counter++;
    log_prod -= log((double)counter);
  }
  //std::cerr << "    Result = " << counter-1 << "\n";
  return counter - 1;
}


#endif //BINSAMPLER_H
