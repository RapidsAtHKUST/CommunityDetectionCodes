/*   (C) Copyright 2004, 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_rand_h
#define tingea_rand_h

#include <stdlib.h>


#define MCX_RAND_MAX RAND_MAX

#define mcxUniform0 ((1.0 * rand()) / ((double) RAND_MAX + 1.0))
#define mcxUniform1 (1.0 - ((1.0 * rand()) / ((double) RAND_MAX + 1.0)))


/*   This is for weak seeding, to obtain fresh seeds which will definitely
 *   *not* be suitable for cryptographic needs
*/

unsigned long mcxSeed
(  unsigned long seedlet
)  ;


double mcxNormal
(  void
)  ;

double mcxNormalCut
(  double radius
,  double stddev
)  ;

double mcxNormalZiggurat
(  void
)  ;

double mcxNormalBoxMuller
(  void
)  ;


/*    Generate numbers in the interval [-outer, outer] according
 *    to the normal distribution with standard deviation sigma.
 *    Use e.g. outer = 2.0 sigma = 0.5
*/

double mcxNormalSample
(  double radius
,  double stddev
)  ;


#endif

