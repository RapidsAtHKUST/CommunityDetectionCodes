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

#ifndef LCE_RANDGENS
#define LCE_RANDGENS
#include<cmath>

#ifndef M_PI /* No gnu? */
#define M_PI 3.1415926535897932384626433832795
#endif


template<typename FloatType=float, bool allowExactZeros=true>
class Ranmar {
private:
  FloatType u[98], c, cd, cm;
  int i97, j97;

public:
  typedef FloatType NativeType;
  typedef int SeedType;
  static const int defaultSeed=54211737;
  Ranmar(int ijkl=defaultSeed) {
    int ij, kl, i, j, k, l, ii, jj, m;
    FloatType s, t;
  
    ij = ijkl / 30082;
    kl = ijkl - 30082 * ij;
    i = (ij / 177) % 177 + 2;
    j = ij % 177 + 2;
    k = (kl / 169) % 178 + 1;
    /* l = kl % 169 + 1; This was the error corrected by J. H. */
    l = kl % 169;
 
    for(ii = 1; ii < 98; ii++) {
      s = 0.0;
      t = 0.5; 
      for(jj = 1; jj < 25; jj++) {
	m = (((i*j) % 179) * k) % 179;
	i = j;
	j = k;
	k = m;
	l = (53*l + 1) % 169;
	if((l * m) % 64 >= 32) s+=t;    
	t *= 0.5;
      }
      u[ii] = s;
    } /* ii */
    c = 362436.0 / 16777216.0;
    cd = 7654321.0 / 16777216.0;
    cm = 16777213.0 / 16777216.0;
    i97 = 97;
    j97 = 33;
    // cerr << "Ranmar inited\n";
  }

  FloatType nextNormed() {return next();}
  
  
  FloatType next() {
    FloatType uni;

    do { /* Exact zero-loop */
      uni = u[i97] - u[j97];
      if(uni < 0) uni += 1.0;
      u[i97] = uni;
      i97 -= 1;
      if(i97 == 0) i97 = 97;
      j97 -= 1;
      if(j97 == 0) j97 = 97;
      c -= cd;
      if(c < 0) c += cm;
      uni -= c;
      if(uni < 0) uni += 1;
    } while (!allowExactZeros && (uni == 0));
    return uni;
  }
};


/**
 * Random value generators with bulk generation ability, and associated 
 * distribution generators. 
 *
 * Implementation: In order to allow several components of the program
 * to use the very same generator instance, we want to give it as a 
 * parameter to the distribution generators. Therefore we cannot use a
 * template for the random number generator proper: this would lead
 * to different instances of the generator itself. As we want to allow 
 * different types of generators, we necessarily have to call the generation
 * function through an abstract interface. The problem is that such 
 * functions cannot be inlined, leading to a significant performance 
 * decrease. The solution is to include bulk generation, filling
 * an array of a given type by sequential random numbers. In this way,
 * only one non-inlined call is needed for the bulk.
 *
 */

template<typename Generator=Ranmar<> >
class RandNumGen:public Generator {
public: 
  typedef typename Generator::NativeType NativeType;
  typedef typename Generator::SeedType SeedType;

  RandNumGen(SeedType seed):
    Generator(seed) {}  

  RandNumGen() {}

  template<typename ResultType> 
  ResultType next(ResultType ceil) {
    return (ResultType) (Generator::nextNormed()*ceil);
  }

  //size_t next(size_t ceil) {
  //return Generator::nextNormed()*ceil;
  //}
  
  template<typename ResultType>
  ResultType nextNative() {
    return Generator::nextNative();
  }

  /**
   * The Box-Müller algorithm for generating normally distributed random 
   * numbers. The results are written on the input values.
   */
  
  template<typename FloatType>
  void boxMuller(FloatType & first, FloatType & second) {
    FloatType sinval;
    FloatType cosval;
    FloatType angle;
    FloatType factor;
    angle=2 * ((FloatType) M_PI) * Generator::nextNormed();
    /* This should be enough for the compler to use a combined 
     * instruction: (BTW. gcc does not yet) */ 
    sinval=sin(angle); cosval=cos(angle);
    factor=sqrt(-2*log(Generator::nextNormed()));
    first=factor*cosval;
    second=factor*sinval;
  }
};

extern RandNumGen<> globalRandSource;

#endif
