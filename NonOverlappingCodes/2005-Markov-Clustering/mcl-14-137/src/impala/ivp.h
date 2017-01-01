/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef impala_ivp_h
#define impala_ivp_h

#include "ivptypes.h"

/* #include "util/alloc.h"
 */
#include "util/types.h"


                        /* Index Value Pair */
typedef struct
{  pnum        idx
;  pval        val
;
}  mclIvp      ; 
                        /* from here on, allow the prefix mclp as shorthand for
                         * mclIvp */

#define mclp mclIvp     /* allow it as shorthand for the type as well */


mclIvp* mclpInstantiate
(  mclIvp*  prealloc_ivp
,  long     idx
,  double   value
)  ;


mclIvp* mclpInit
(  mclIvp*  ivp
)  ;


void*  mclpInit_v
(  void*    ivp
)  ;


mclIvp* mclpCreate
(  long     idx
,  double   value
)  ;



   /* for use as callback e.g. in mcxMedian */

double mclpGetDouble
(  const void* ivp
)  ;


void mclpFree
(  mclIvp**   ivp
)  ;


/* arg should be of type double */

mcxbool mclpGivenValGQ
(  mclIvp*        ivp
,  void*          arg
)  ;

mcxbool mclpGivenValLQ
(  mclIvp*        ivp
,  void*          arg
)  ;


int mclpIdxGeq
(  const void*             ivp1
,  const void*             ivp2
)  ;


int mclpIdxCmp
(  const void*             ivp1
,  const void*             ivp2
)  ;


int mclpIdxRevCmp
(  const void*             ivp1
,  const void*             ivp2
)  ;


int mclpValCmp
(  const void*             ivp1
,  const void*             ivp2
)  ;


int mclpValRevCmp
(  const void*             ivp1
,  const void*             ivp2
)  ;


/* discard ivp2 */
void mclpMergeLeft
(  void*                   ivp1
,  const void*             ivp2
)  ;


/* discard ivp1 */
void mclpMergeRight
(  void*                   ivp1
,  const void*             ivp2
)  ;


void mclpMergeAdd
(  void*                   ivp1
,  const void*             ivp2
)  ;


void mclpMergeMax
(  void*                   ivp1
,  const void*             ivp2
)  ;


void mclpMergeMin
(  void*                   ivp1
,  const void*             ivp2
)  ;


void mclpMergeMul
(  void*                   ivp1
,  const void*             ivp2
)  ;

#define MCLPAR_SORTED         1
#define MCLPAR_UNIQUE         2

typedef struct
{  mclIvp*     ivps
;  dim         n_ivps
;  dim         n_alloc
;  mcxbits     sorted
;
}  mclpAR   ;


mclpAR* mclpARinit
(  mclpAR* mclpar
)  ;


void* mclpARinit_v
(  void* mclpar
)  ;


mclpAR* mclpARensure
(  mclpAR*  mclpar
,  dim      n
)  ;


mcxbool mclpARbatchCheck
(  mclpAR* ar
,  long range_lo
,  long range_hi
)  ;


mcxstatus mclpARextend
(  mclpAR*  ar
,  long     idx
,  double   val
)  ;


void mclpARreset
(  mclpAR*  ar
)  ;

mclpAR* mclpARfromIvps
(  mclpAR*  mclpar
,  mclp*    ivps
,  dim      n
)  ;


void mclpARfree
(  mclpAR**    mclparp
)  ;  


double mclpUnary
(  mclp*    ivp
,  mclpAR*  ar       /* idx: MCLX_UNARY_mode, val: arg */
)  ;



typedef struct
{  double*     lft
;  double*     rgt
;  mcxbits     equate   /* 1: lq, 2: gq */
;
}  mclpVRange   ;


mcxbool mclpSelectValues
(  mclp     *ivp
,  void     *range
)  ;


typedef struct
{  long*       lft
;  long*       rgt
;  mcxbits     equate   /* 1: lq, 2: gq */
;
}  mclpIRange   ;

mcxbool mclpSelectIdcs
(  mclp     *ivp
,  void     *range
)  ;


typedef struct
{  pnum  src
;  pnum  dst
;  pnum  val
;
}  mclEdge;

#define mcle mclEdge


int mcleCmp
(  const void* e1
,  const void* e2
)  ;

int mcleSrcCmp
(  const void* e1
,  const void* e2
)  ;

int mcleDstCmp
(  const void* e1
,  const void* e2
)  ;


#endif

