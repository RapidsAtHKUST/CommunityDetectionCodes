/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <pthread.h>

#include "proc.h"
#include "inflate.h"
#include "dpsd.h"
#include "expand.h"

#include "impala/io.h"
#include "impala/matrix.h"

#include "util/ting.h"
#include "util/err.h"
#include "util/io.h"
#include "util/types.h"
#include "util/alloc.h"


typedef struct
{  int            id
;  int            start
;  int            end
;  double         power
;  mclMatrix*     mx
;
}  mclvInflateLine_arg   ;


void mclxInflateBoss
(  mclMatrix*        mx
,  double            power
,  mclProcParam*     mpp
)
   {  int            workLoad    =  N_COLS(mx) / mpp->n_ithreads
   ;  int            workTail    =  N_COLS(mx) % mpp->n_ithreads
   ;  int            i           =  0
   ;  pthread_attr_t pthread_custom_attr

   ;  pthread_t *threads_inflate
      =  (pthread_t *) mcxAlloc
         (  mpp->n_ithreads*sizeof(pthread_t)
         ,  EXIT_ON_FAIL
         )

   ;  pthread_attr_init(&pthread_custom_attr)

   ;  for (i=0;i<mpp->n_ithreads;i++)
      {
         mclvInflateLine_arg *a
                     =  (mclvInflateLine_arg *)
                        malloc(sizeof(mclvInflateLine_arg))
      ;  a->id       =  i
      ;  a->start    =  workLoad * i
      ;  a->end      =  workLoad * (i+1)
      ;  a->mx       =  mx
      ;  a->power    =  power

      ;  if (i+1==mpp->n_ithreads)
         a->end   +=  workTail

      ;  pthread_create
         (  &threads_inflate[i]
         ,  &pthread_custom_attr
         ,  (void *(*)(void*)) mclvInflateLine
         ,  (void *) a
         )
   ;  }

      for (i = 0; i < mpp->n_ithreads; i++)
      pthread_join(threads_inflate[i], NULL)

   ;  mcxFree(threads_inflate)
;  }


void  mclvInflateLine
(  void *arg
)
   {  mclvInflateLine_arg *a=  (mclvInflateLine_arg *)  arg
   ;  mclMatrix*     mx          =  a->mx
                     
   ;  mclVector*     vecPtr      =  mx->cols + a->start
   ;  mclVector*     vecPtrMax   =  mx->cols + a->end
   ;  double         power       =  a->power

   ;  while (vecPtr < vecPtrMax)
      {  mclvInflate(vecPtr, power)
      ;  vecPtr++
   ;  }
      free(a)
;  }


