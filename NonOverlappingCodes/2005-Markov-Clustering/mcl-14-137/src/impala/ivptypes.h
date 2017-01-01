/*   (C) Copyright 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2013 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef impala_ivptypes_h
#define impala_ivptypes_h

#include <limits.h>
#include <float.h>

#ifdef VALUE_AS_DOUBLE
      typedef  double pval;
#  define   PVAL_MAX       DBL_MAX
#  define   PVAL_MIN       DBL_MIN
#  define   PVAL_EPSILON   DBL_EPSILON
#  define   IVP_VAL_TYPE "double"
#else
      typedef float pval;
#  define   PVAL_MAX       FLT_MAX
#  define   PVAL_MIN       FLT_MIN
#  define   PVAL_EPSILON   FLT_EPSILON
#  define   IVP_VAL_TYPE "float"
#endif


/* pnum must be signed; -1 is used as special value - although
 * in very few places only.
*/
#ifdef INDEX_AS_LONG
      typedef  long  pnum;
#  define   PNUM_MAX    LONG_MAX
#  define   PNUM_MIN    LONG_MIN
#  define   IVP_NUM_TYPE "long"
#else
      typedef  int  pnum;
#  define   PNUM_MAX    INT_MAX
#  define   PNUM_MIN    INT_MIN
#  define   IVP_NUM_TYPE "int"
#endif


#endif

