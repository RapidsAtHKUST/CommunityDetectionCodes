/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

/* include <stdio.h>
   include <stdlib.h>
   include <string.h>
   include <sys/types.h>
*/


#if 0
#include "gperftools/tcmalloc.h"
#  define  themalloc   tc_malloc
#  define  therealloc  tc_realloc
#  define  thefree     tc_free
#else
#  define  themalloc   malloc
#  define  therealloc  realloc
#  define  thefree     free
#endif


#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "alloc.h"
#include "err.h"


static dim    mcx_alloc_maxchunksize  =  1048576;
static long    mcx_alloc_maxtimes      =  -1;
static mcxbool mcx_alloc_limit         =  FALSE;


void mcxAllocLimits
(  long  maxchunksize
,  long  maxtimes
)
   {  if (maxchunksize > 0)
      mcx_alloc_maxchunksize = maxchunksize
   ;  if (maxtimes > 0)
      mcx_alloc_maxtimes = maxtimes
   ;  mcx_alloc_limit = TRUE;
;  }


void* mcxRealloc
(  void*             object
,  dim               new_size
,  mcxOnFail         ON_FAIL
)
   {  void*          mblock   =  NULL
   ;  mcxstatus      status   =  STATUS_OK
   ;  const char*    me       =  "mcxRealloc"

   ;  if (!new_size)
      {  if (object)
         mcxFree(object)
   ;  }
      else
      {  if
         (  mcx_alloc_limit
         && (!mcx_alloc_maxtimes-- || new_size > mcx_alloc_maxchunksize)
         )
         mblock = NULL
      ;  else
         mblock = object
               ?  therealloc(object, new_size)
               :  themalloc(new_size)
   ;  }

      if (new_size && (!mblock))
         mcxMemDenied(stderr, me, "byte", new_size)
      ,  status   =  1
   ;
      if (status)
      {  const char* tin = getenv("TINGEA_MEM_SIGNAL")
      ;  if (tin)
         {  unsigned tintin = atoi(tin)
         ;  raise(tintin ? tintin : SIGSEGV)
      ;  }
         if (ON_FAIL == SLEEP_ON_FAIL)
         {  mcxTell(me, "pid %ld, entering sleep mode", (long) getpid())
         ;  while(1)
            sleep(1000)
      ;  }
         if (ON_FAIL == EXIT_ON_FAIL || ON_FAIL == ENQUIRE_ON_FAIL)
         {  mcxTell(me, "going down")
         ;  if (ON_FAIL == ENQUIRE_ON_FAIL)
            mcxTell(me, "ENQUIRE fail mode ignored")
         ;  exit(1)
      ;  }
      }

      return mblock
;  }


void mcxNFree
(  void*             base
,  dim               n_elem
,  dim               elem_size
,  void            (*obRelease) (void *)
)
   {  if (n_elem && obRelease)
      {  char *ob    =  base
      ;  while (n_elem-- > 0)
            obRelease(ob)
         ,  ob += elem_size
   ;  }
      mcxFree(base)
;  }


void* mcxNAlloc
(  dim               n_elem
,  dim               elem_size
,  void*           (*obInit) (void *)
,  mcxOnFail         ON_FAIL
)
   {  return mcxNRealloc(NULL, n_elem, 0, elem_size, obInit, ON_FAIL)
;  }


void* mcxNRealloc
(  void*             mem
,  dim               n_elem
,  dim               n_elem_prev
,  dim               elem_size
,  void*           (*obInit) (void *)
,  mcxOnFail         ON_FAIL
)
   {  char*    ob
   ;  mem =  mcxRealloc(mem, n_elem * elem_size, ON_FAIL)
   ;
      if (!mem)
      return NULL

   ;  if (obInit && n_elem > n_elem_prev)
      {  ob  =  ((char*) mem) + (elem_size * n_elem_prev)
      ;  while (n_elem-- > n_elem_prev)      /* careful with unsignedness */
         {  obInit(ob)
         ;  ob += elem_size
      ;  }
      }

      return mem
;  }


void mcxMemDenied
(  FILE*             channel
,  const char*       requestee
,  const char*       unittype
,  dim               n
)
   {  mcxErrf
      (  channel
      ,  requestee
      ,  "memory shortage: could not alloc [%lu] instances of [%s]"
      ,  (ulong) n
      ,  unittype
      )
;  }


void mcxFree
(  void* object
)
   {  if (object) thefree(object)
;  }


void* mcxAlloc
(  dim               size
,  mcxOnFail         ON_FAIL
)
   {  return mcxRealloc(NULL, size, ON_FAIL)
;  }


