/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* Severely underdocumented.
*/

#ifndef impala_label_h
#define impala_label_h

#include "vector.h"
#include "matrix.h"

#include "util/types.h"
#include "util/io.h"
#include "util/hash.h"


typedef struct
{  mclv*       domain
;  char**      labels   /* size: domain->n_ivps+1, also NULL terminated */
;  mcxTing*    na       /* not available, returned if element not found */
;
}  mclTab       ;

#define N_TAB(tab) ((tab)->domain->n_ivps)
#define TAB_IS_NA(tab, label) (label == (tab)->na->str)



/* If dom is nonNULL, demand equality. TODO: allow subsumption.
*/

mclTab* mclTabRead
(  mcxIO*         xf
,  const mclv*    dom
,  mcxOnFail      ON_FAIL
)  ;


mcxstatus mclTabWrite
(  mclTab*        tab
,  mcxIO*         xf
,  const mclv*    select   /* if NULL, use all */
,  mcxOnFail      ON_FAIL
)  ;



/* write <num> <num> in tab. fixname
*/

mcxstatus mclTabWriteDomain
(  mclv*          select
,  mcxIO*         xfout
,  mcxOnFail      ON_FAIL
)  ;


char* mclTabGet
(  const mclTab*  tab
,  long     id
,  long*    ofs
)  ;


/*    Assumes tings as keys; integer encoded in kv->val.
 *    labels in map are shallow pointer copies of (kv->key)->str.  When freeing
 *    hash, *do* free the ting, do not free the str!  - use mcxTingAbandon
 *    callback.
 *
 *    Quite awful dependencies.
 *    Future: remove this (implementation and/or (entire) interface).
 *    TODO write hash freeing routine (duh).
*/

mclTab* mclTabFromMap
(  mcxHash* map
)  ;

mcxHash* mclTabHash
(  mclTab* tab
)  ;

void mclTabHashSet
(  mcxHash* hash
,  ulong u
)  ;

void mclTabFree
(  mclTab**       tabpp
)  ;


/* precondition:  TRUE == mcldEquate(tab->domain, map->dom_cols, MCLD_EQT_SUB)
 * and map is a bijection.
 *
 * postcondition: tab->domain = map(tab->domain)
 *
 * invokes mclvUnionv with SCRATCH_READY.
*/

mclTab* mclTabMap
(  const mclTab*  tab
,  mclx*          map
)  ;



mclTab* mclTabSelect
(  const mclTab*  tab
,  const mclv*    select
)  ;

mclx* mclTabDuplicated
(  mclTab* tab
,  mcxHash** h
)  ;

#endif

