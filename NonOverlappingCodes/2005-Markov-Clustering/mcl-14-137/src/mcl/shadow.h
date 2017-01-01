/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* This is all experimental and explorative code. So far plain vanilla mcl
 * seems to be best.
*/


#ifndef mcl_shadow_h
#define mcl_shadow_h

#include "util/ting.h"
#include "impala/matrix.h"

#define  MCL_SHADOW_EARLY        1 <<  0
#define  MCL_SHADOW_E_HIGH       1 <<  1
#define  MCL_SHADOW_E_LOW        1 <<  2
#define  MCL_SHADOW_V_HIGH       1 <<  3
#define  MCL_SHADOW_V_LOW        1 <<  4

#define  MCL_SHADOW_MULTIPLY     1 << 10
#define  MCL_SHADOW_SELF         1 << 11

#define  MCL_SHADOW_TRY          1 << 14


   /* It is called 'turtle' to indicate that this is one particular
    * and arbitrarily named strategy of obtaining shadowing factors.
   */
mclv* mcl_get_shadow_turtle_factors
(  const mclx* mx
,  long shadow_mode
,  double shadow_s
)  ;


mclv* mcl_shadow_matrix
(  mclx* mx
,  const mclv* factors
)  ;

mclv* mcl_density_adjust
(  mclx* mx
,  const char* da
)  ;

#endif


