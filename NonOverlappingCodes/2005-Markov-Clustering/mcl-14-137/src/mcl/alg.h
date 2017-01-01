/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/*    NOTE. the interfaces in this library willfully violate encapsulation
 *    principles. The rationale is that they handle huge objects; the caller
 *    can stipulate which ones should be cached. This is done in a very
 *    simple manner, with the cached objects accessible in the parameter
 *    object, and the caller instructing mclAlgorithm with ALG_CACHE_XXX
 *    flags.
 *
 *    All the same, the current situation can and will be incrementally
 *    improved.
*/

#ifndef mcl_alg_h
#define mcl_alg_h

#include "impala/matrix.h"

#include "util/opt.h"
#include "util/hash.h"

#include "proc.h"
#include "transform.h"
#include "impala/tab.h"


#define ALG_OPT_INFO MCX_OPT_UNUSED

extern mcxOptAnchor mclAlgOptions[];
extern int level_quiet;


typedef struct
{  mcxIO*               xfout
;  mclProcParam*        mpp

;  int                  expandDigits

;  double               pre_inflation
;  double               pre_inflationx
;  mcxbool              foundOverlap

#  define   ALG_DO_APPEND_LOG          1  <<  0
#  define   ALG_DO_ANALYZE             1  <<  1
#  define   ALG_DO_SHOW_LOG            1  <<  3
#  define   ALG_DO_CHECK_CONNECTED     1  <<  6
#  define   ALG_DO_FORCE_CONNECTED     1  <<  7
#  define   ALG_DO_OUTPUT_LIMIT        1  <<  8

#  define   ALG_DO_IO                  1  << 10
#  define   ALG_CACHE_INPUT            1  << 11
#  define   ALG_CACHE_START            1  << 12
#  define   ALG_CACHE_EXPANDED         1  << 13
#  define   ALG_DO_DISCARDLOOPS        1  << 14
#  define   ALG_DO_SHADOW              1  << 15
#  define   ALG_DO_SHOW_PID            1  << 16
#  define   ALG_DO_SHOW_JURY           1  << 17
#  define   ALG_DO_SUMLOOPS            1  << 18

;  mcxbits              modes

;  mcxbits              stream_modes
;  mcxbool              stream_write_labels
;  mclTab*              tab
;  mcxTing*             fn_write_input
;  mcxTing*             fn_write_start
;  mcxTing*             fn_read_tab

;  mcxTing*             fnicl
;  mcxTing*             stream_transform_spec
;  mclgTF*              stream_transform

;  mcxTing*             transform_spec
;  mclgTF*              transform
;  mcxbool              adjust_loops
;  mclv*                shadow_cache_domain   /* cache original domain when shadowing */
;  long                 shadow_mode
;  double               shadow_s
;  double               center
;  mcxbool              expand_only

;  mclx*                mx_input
;  mclx*                mx_start
;  mclv*                mx_start_sums
;  mclx*                mx_expanded
;  mclx*                mx_limit

;  mclx*                cl_result
;  mclx*                cl_assimilated
;  dim                  n_assimilated

;  int                  write_mode
;  int                  sort_mode
;  int                  overlap_mode
;  mcxTing*             cline
;  mcxTing*             fnin
;
}  mclAlgParam          ;



/*
 * This is a work in progress. The current interface is
 * not yet satisfactorily separated into logical parts.
 *
 * When modes & ALG_DO_CACHE is set mx_start will *not*
 * be deleted. Similar for other modes.
*/

mcxstatus mclAlgInterface
(  mclAlgParam** mlppp
,  char ** argv2
,  int argc2
,  const char* fn_input       /* set either this */
,  mclx*       mx_input       /* or this */
,  mcxbits modes
)  ;


/*    purposely half-hearted attempt at crappy encapsulation.
 *    it is convenience + encapsulation reminder.
*/

void* mclAlgParamRelease
(  mclAlgParam *mlp
,  void*       what
)  ;


void mclAlgParamFree
(  mclAlgParam** app
,  mcxbool free_composites
)  ;


enum
{  ALG_INIT_OK    =  0
,  ALG_INIT_DONE
,  ALG_INIT_FAIL
}  ;

/* returns one of the above */

mcxstatus mclAlgorithmInit
(  const mcxOption* opts
,  mcxHash*       hashedOpts
,  const char*    fname
,  mclAlgParam*   mlp
)  ;

void mclAlgOptionsInit
(  void
)  ;

mcxstatus mclAlgorithm
(  mclAlgParam*   mlp
)  ;

mcxstatus mclAlgorithmStart
(  mclAlgParam* mlp
,  mcxbool reread
)  ;

#endif

