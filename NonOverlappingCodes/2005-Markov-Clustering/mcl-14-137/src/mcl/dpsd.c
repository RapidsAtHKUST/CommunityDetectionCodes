/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "dpsd.h"
#include "interpret.h"

#include "impala/ivp.h"
#include "util/minmax.h"
#include "util/alloc.h"
#include "util/types.h"

double   dpsd_delta            =     12 * FLT_EPSILON;

mclMatrix* mclDiagOrdering
(  const mclMatrix*     M
,  mclVector**          vecp_attr
)
   {  int         n_cols      =  N_COLS(M)
   ;  mclMatrix*  diago       =  mclxAllocZero(NULL, NULL)
   ;  long        col

   ;  if (*vecp_attr != NULL)
      mclvFree(vecp_attr)

   ;  *vecp_attr = mclvResize(NULL, n_cols)

   ;  for (col=0;col<n_cols;col++)
      {  ofs      offset      =  -1
      ;  double   selfval     =  mclvIdxVal(M->cols+col, col, &offset)
      ;  double   center      =  mclvPowSum(M->cols+col, 2.0)
     /*  double   maxval      =  mclvMaxValue(M->cols+col)
      */
      ;  double   bar         =  MCX_MAX(center, selfval) - dpsd_delta
      ;  mclIvp*  ivp         =  (*vecp_attr)->ivps+col

      ;  ivp->idx             =  col
      ;  ivp->val             =  center ? selfval / center : 0

      ;  if (offset >= 0)                 /* take only higher valued entries */
         mclvSelectGqBar(diago->cols+col, bar)
   ;  }
   ;  return diago
;  }



