/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "util/minmax.h"
#include "util/compile.h"
#include "util/rand.h"
#include "pval.h"

double fltxConst
(  pval     p_unused
,  void*    arg
)
   {  return *((double*)arg)
;  }


double fltxGQ
(  pval     d
,  void*    arg
)
   {  return d >= *((double*)arg) ? d : 0.0
;  }


double fltxGT
(  pval     d
,  void*    arg
)
   {  return d > *((double*)arg) ? d : 0.0
;  }


double fltxLT
(  pval     d
,  void*    arg
)
   {  return d < *((double*)arg) ? d : 0.0
;  }


double fltxPositive
(  pval     d
,  void*    arg
)
   {  return d > 0 ? d : 0
;  }


double fltxLQ
(  pval     d
,  void*    arg
)
   {  return d <= *((double*)arg) ? d : 0.0
;  }


double fltxCopy
(  pval     flt
,  void*    arg_unused
)
   {  return flt
;  }


double fltxScale
(  pval     d
,  void*    arg
)
   {  double e = *((double*) arg)
   ;  return e ? d/e : 0.0
;  }


double fltxMul
(  pval     d
,  void*    arg
)
   {  return d * (*((double*)arg))
;  }


double fltxRand
(  pval     flt
,  void*    pbb_keep
)
   {  long d = rand()
   ;  return (d / (double) RAND_MAX) <= *((double*)pbb_keep) ? flt : 0.0
;  }


double fltxAdd
(  pval     flt
,  void     *add
)
   {  return flt + *((double*) add)
;  }


double fltxPower
(  pval     flt
,  void     *power
)
   {  return pow(flt, *((double*) power))
;  }


double fltxLog
(  pval     flt
,  void*    basep
)
   {  double base = basep ? *((double*) basep) : -1
   ;  if (base > 0 && flt > 0)
      return log(flt) / log(base)
   ;  else if ((!base || !basep) && flt > 0)
      return log(flt)
   ;  else if (!flt)
      return -FLT_MAX
   ;  else
      return 0.0
;  }


double fltxAbs
(  pval     flt
,  void*    arg_unused cpl__unused
)
   {  return flt > 0 ? flt : -flt
;  }


double fltxNeglog
(  pval     flt
,  void*    base
)
   {  return -fltxLog(flt, base)
;  }


double fltxExp
(  pval     power
,  void*    flt
)
   {  double base = *((double*) flt)
   ;  if (!base)
      return exp(power)
   ;  else
      return pow(*((double*) flt), power)
;  }


double fltxCeil
(  pval     flt
,  void*    val
)
   {  return flt > *((double*) val) ? *((double*) val) : flt
;  }


double fltxFloor
(  pval     flt
,  void*    val
)
   {  return flt < *((double*) val) ? *((double*) val) : flt
;  }


double fltxAcos
(  pval     flt
,  void*    val
)
   {  return acos(flt > 1.0 ? 1.0 : flt < -1.0 ? -1.0 : flt)
;  }


double fltxPropagateMax
(  pval     d
,  void*    arg
)
   {  if (d > *((double*)arg))
      *((double*)arg) = d
   ;  return d
;  }


double fltLeft
(  pval     d1
,  pval     arg_unused
)
   {  return d1
;  }


double fltRight
(  pval     p_unused
,  pval     d2
)
   {  return d2
;  }


double fltAdd
(  pval     d1
,  pval     d2
)
   {  return d1 + d2
;  }


double flt0p0
(  pval     p_unused1
,  pval     p_unused2
)
   {  return 0.0
;  }


double flt0p5
(  pval     p_unused1
,  pval     p_unused2
)
   {  return 0.5
;  }


double flt1p0
(  pval     p_unused1
,  pval     p_unused2
)
   {  return 1.0
;  }


double flt1p5
(  pval     p_unused1
,  pval     p_unused2
)
   {  return 1.5
;  }


double fltSubtract
(  pval     d1
,  pval     d2
)
   {  return d1 - d2
;  }


double fltMultiply
(  pval     d1
,  pval     d2
)
   {  return  d1 * d2
;  }


double fltCross
(  pval     d1
,  pval     d2
)
   {  return  d1 && d2 ? d1 * d2 : d1 ? d1 : d2
;  }


double fltMin
(  pval     d1
,  pval     d2
)
   {  return (d1 < d2) ? d1 : d2
;  }


double fltMax
(  pval     d1
,  pval     d2
)
   {  return (d1 > d2) ? d1 : d2
;  }


double fltMinNZ
(  pval     d1
,  pval     d2
)
   {  return (d1 && d2) ? fltMin(d1, d2) : 0.0
;  }


double fltMaxNZ
(  pval     d1
,  pval     d2
)
   {  return (d1 && d2) ? fltMax(d1, d2) : 0.0
;  }


double fltLoR
(  pval     lft
,  pval     rgt
)
   {  return lft ? lft : rgt
;  }


double fltLaNR
(  pval     lft
,  pval     rgt
)
   {  return rgt ? 0.0 : lft
;  }


double fltLaR
(  pval     lft
,  pval     rgt
)
   {  return lft && rgt ? lft : 0.0
;  }



double fltArcMaxGQ(pval lft, pval rgt, pval arg) {  return lft >= arg || rgt >= arg ?  lft :  0.0 ;  }
double fltArcMaxGT(pval lft, pval rgt, pval arg) {  return lft >  arg || rgt >  arg ?  lft :  0.0 ;  }
double fltArcMaxLQ(pval lft, pval rgt, pval arg) {  return lft <= arg || rgt <= arg ?  lft :  0.0 ;  }
double fltArcMaxLT(pval lft, pval rgt, pval arg) {  return lft <  arg || rgt <  arg ?  lft :  0.0 ;  }

double fltArcMinGQ(pval lft, pval rgt, pval arg) {  return lft >= arg && rgt >= arg ?  lft :  0.0 ;  }
double fltArcMinGT(pval lft, pval rgt, pval arg) {  return lft >  arg && rgt >  arg ?  lft :  0.0 ;  }
double fltArcMinLQ(pval lft, pval rgt, pval arg) {  return lft <= arg && rgt <= arg ?  lft :  0.0 ;  }
double fltArcMinLT(pval lft, pval rgt, pval arg) {  return lft <  arg && rgt <  arg ?  lft :  0.0 ;  }

double fltArcDiffGQ(pval lft, pval rgt, pval arg) {  return lft - rgt >= arg || rgt - lft >= arg ?  lft :  0.0 ;  }
double fltArcDiffGT(pval lft, pval rgt, pval arg) {  return lft - rgt >  arg || rgt - lft >  arg ?  lft :  0.0 ;  }
double fltArcDiffLQ(pval lft, pval rgt, pval arg) {  return lft - rgt <= arg || rgt - lft <= arg ?  lft :  0.0 ;  }
double fltArcDiffLT(pval lft, pval rgt, pval arg) {  return lft - rgt <  arg || rgt - lft <  arg ?  lft :  0.0 ;  }


double fltArcMax
(  pval     lft
,  pval     rgt
)
   {  return
         lft && rgt
      ?  (  lft >= rgt ? lft : 0.0 )
      :  lft
;  }

