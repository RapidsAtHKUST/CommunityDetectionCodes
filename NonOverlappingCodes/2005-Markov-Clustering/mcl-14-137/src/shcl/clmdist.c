/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010 Stijn van Dongen
 *   (C) Copyright 2011, 2012, 2013  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* TODO
 *    rand index, randc (Hubert et ??)

   (n:2) + 2 Sum{i=1->c1} Sum{j=1->c2} (n_ij:2) - [ Sum{i=1->c1} (n_i+:2) + Sum{j=1->c2} (n_+j:2) ]
                                 -------------------------
                                       n (n-1) : 2
     a + d
   ---------
     (n/2)

   a: same in C1 and same in C2
   d: different in C1 and different in C2.


            Sum{i=1->c1} Sum{j=1->c2} (n_ij/2) - [1/(n/2)] Sum{i=1->c1} (n_i/2) * Sum{j=1->c2} (n_j/2)
                                 -------------------------
 [1/2] *  ( Sum{i=1->c1} (n_i/2) + Sum{j=1->c2} (n_j/2) ) - [1/(n/2)] Sum{i=1->c1} (n_i/2) * Sum{j=1->c2} (n_j/2)

*/

#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "clm.h"
#include "report.h"
#include "clmdist.h"

#include "impala/matrix.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/ivp.h"
#include "impala/app.h"
#include "impala/io.h"

#include "clew/clm.h"
#include "clew/cat.h"

#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"



static double mclv_choose_sum
(  const mclv* vec
)  
   {  mclIvp* vecivps = vec->ivps
   ;  dim     vecsize = vec->n_ivps
   ;  double  sum  = 0.0

   ;  while (vecsize-- > 0)
      {  double val = (vecivps++)->val
      ;  double delta = val * (val - 1.0) * 0.5
      ;  if (delta > 0)
         sum += delta
   ;  }
      return sum
;  }


static double mclx_choose_sum
(  const mclx* mx
)
   {  dim i
   ;  double sum = 0.0
   ;  for (i=0;i<N_COLS(mx);i++)
      sum += mclv_choose_sum(mx->cols+i)
   ;  return sum
;  }


enum
{  DIST_SPLITJOIN 
,  DIST_VARINF
,  DIST_MIRKIN
,  INDEX
}  ;


/*
 * clmdist will enstrict the clusterings, and possibly project them onto
 * the meet of the domains if necessary.
*/


static const char* me = "clm dist";


enum
{  DIST_OPT_OUTPUT = CLM_DISP_UNUSED
,  DIST_OPT_MODE
,  DIST_OPT_INDEX
,  DIST_OPT_CHAIN
,  DIST_OPT_WHEEL
,  DIST_OPT_SORT
,  DIST_OPT_NORMALISE
,  DIST_OPT_MCI
,  DIST_OPT_DIGITS
,  DIST_OPT_FACTOR
}  ;


enum
{  VOL_OPT_OUTPUT = CLM_DISP_UNUSED
,  VOL_OPT_FACTOR
}  ;

static mcxOptAnchor volOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  VOL_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-fraction"
   ,  MCX_OPT_HASARG
   ,  VOL_OPT_FACTOR
   ,  "<num>"
   ,  "stringency factor: require |meet| >= <num> * |cls-size| (default 0.5)"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxOptAnchor distOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  DIST_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-mode"
   ,  MCX_OPT_HASARG
   ,  DIST_OPT_MODE
   ,  "<mode>"
   ,  "one of sj|vi|mirkin"
   }
,  {  "--mci"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  DIST_OPT_MCI
   ,  NULL
   ,  "output all against all matrix of distances"
   }
,  {  "--index"
   ,  MCX_OPT_DEFAULT
   ,  DIST_OPT_INDEX
   ,  NULL
   ,  "output Rand, corrected Rand and Jaccard index"
   }
,  {  "-digits"
   ,  MCX_OPT_HASARG
   ,  DIST_OPT_DIGITS
   ,  "<num>"
   ,  "number of trailing digits for floats"
   }
,  {  "--chain"
   ,  MCX_OPT_DEFAULT
   ,  DIST_OPT_CHAIN
   ,  NULL
   ,  "only compare consecutive clusterings"
   }
,  {  "--one-to-many"
   ,  MCX_OPT_DEFAULT
   ,  DIST_OPT_WHEEL
   ,  NULL
   ,  "compare first clustering against all the rest"
   }
,  {  "--sort"
   ,  MCX_OPT_DEFAULT
   ,  DIST_OPT_SORT
   ,  NULL
   ,  "sort from coarse to fine-grained before computing distances"
   }
,  {  "--normalise"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN    /* not implemented */
   ,  DIST_OPT_NORMALISE
   ,  NULL
   ,  "NOT IMPLEMENTED normalise criterion (applicable for --vi and default split/join)"
   }
,  {  "-fraction"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  DIST_OPT_FACTOR
   ,  "<num>"
   ,  "stringency factor: require |meet| >= <num> * |cls-size| (default 0.5)"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;
static int digits       =  -1;
static int mode_g       =  -1;
static mcxbool i_am_vol =  FALSE;   /* node faithfulness */
static double  nff_fac  =  FLT_MAX;
static mcxbool consecutive_g = -1;
static mcxbool mci_g = -1;
static mcxbool wheel_g = -1;
static mcxbool split_g = -1;
static mcxbool sort_g = -1;


static mcxstatus distInit
(  void
)
   {  xfout =  mcxIOnew("-", "w")
   ;  mode_g = 0
   ;  digits = 2
   ;  nff_fac     =  0.5
   ;  consecutive_g = FALSE
   ;  mci_g = FALSE
   ;  wheel_g = FALSE
   ;  sort_g = FALSE
   ;  split_g = FALSE
   ;  return STATUS_OK
;  }


static mcxstatus volInit
(  void
)
   {  xfout       =  mcxIOnew("-", "w")
   ;  i_am_vol    =  TRUE
   ;  nff_fac     =  0.5
   ;  consecutive_g = FALSE
   ;  mci_g = FALSE
   ;  wheel_g = FALSE
   ;  sort_g = FALSE
   ;  return STATUS_OK
;  }


static mcxstatus volArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case VOL_OPT_FACTOR
      :  nff_fac = atof(val)
      ;  break
      ;

         case VOL_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus distArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case DIST_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         case DIST_OPT_DIGITS
      :  digits = atoi(val)
      ;  break
      ;

         case DIST_OPT_WHEEL
      :  wheel_g = TRUE
      ;  break
      ;

         case DIST_OPT_CHAIN
      :  consecutive_g = TRUE
      ;  break
      ;

         case DIST_OPT_MCI
      :  mci_g = TRUE
      ;  break
      ;

         case DIST_OPT_SORT
      :  sort_g = TRUE
      ;  break
      ;

         case DIST_OPT_FACTOR
      :  nff_fac = atof(val)
      ;  break
      ;

         case DIST_OPT_MODE
      :     if (!strcmp(val, "sj"))
            mode_g = DIST_SPLITJOIN
         ;  else if (!strcmp(val, "vi"))
            mode_g = DIST_VARINF
         ;  else if
            (  !strcmp(val, "ehd")
            || !strcmp(val, "mk")
            || !strcmp(val, "mirkin")
            )
            mode_g = DIST_MIRKIN
         ;  else
            mcxDie(1, me, "unknown mode <%s>", val)
      ;  break
      ;

         case DIST_OPT_INDEX
      :  mode_g = INDEX
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus distMain
(  int                  argc
,  const char*          argv[]
)
   {  int               i
   ;  int a             =  0
   ;  mclx* nff_scores  =  NULL
   ;  mcxIO* xfin       =  mcxIOnew("-", "r")
   ;  mcxbits bits      =  MCLX_PRODUCE_PARTITION | MCLX_REQUIRE_DOMSTACK

   ;  mclxCat st
   ;  mclxCat st2

   ;  mclxCat *stptr = &st, *stptr1 = &st, *stptr2 = &st

   ;  mcxIO* xfdebug = mcxIOnew("-", "w")
   ;  mcxIOopen(xfdebug, EXIT_ON_FAIL)

   ;  mclxCatInit(&st)
   ;  mclxCatInit(&st2)

   ;  if (i_am_vol)
      me = "clm vol"
      
   ;  if (!mode_g)
      mode_g = DIST_SPLITJOIN

   ;  if (mode_g & DIST_MIRKIN || mode_g & DIST_SPLITJOIN)
      digits = 0

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  for (a=0;a<argc;a++)
      {  mcxstatus status
      ;  if (!strcmp(argv[a], "--"))
         {  stptr = &st2
         ;  split_g = TRUE
         ;  stptr2 = &st2
         ;  continue
      ;  }
         mcxIOnewName(xfin, argv[a])
      ;  status = mclxCatRead(xfin, stptr, 0, NULL, NULL, bits)
      ;  mcxIOclose(xfin)
      ;  if (status)
         break
;if (0)
         mclxWrite(stptr->level[stptr->n_level-1].mx, xfdebug, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  }

      if (!a || a != argc)
      mcxDie(1, me, "failed to read one or more cluster files")

   ;  mcxIOfree(&xfdebug)

   ;  if (sort_g)
      mclxCatSortCoarseFirst(&st)

   ;  if (i_am_vol && st.n_level)
      nff_scores
      =  mclxCartesian
         (  mclvCanonical(NULL, 1, 1.0)
         ,  mclvClone(st.level[0].mx->dom_rows)
         ,  1.0
         )

   ;  if (mci_g)

   ;  for (i=0;i<stptr1->n_level;i++)
      {  mclx* c1       =  stptr1->level[i].mx
      ;  int j, jstart  =  split_g ? 0 : i+ 1
      ;  for (j=jstart; j<stptr2->n_level;j++)
         {  mclx* c2  =  stptr2->level[j].mx
         ;  mclx* meet12, *meet21
         ;  double dist1d, dist2d
         ;  dim dist1i, dist2i
         ;  dim n_volatile = 0
         ;  dim nff[5] = { 0, 0, 0, 0, 0 }

         ;  meet12 =  clmContingency(c1, c2)
         ;  meet21 =  mclxTranspose(meet12)

         ;  {  dim k
            ;  for (k=0;k<N_COLS(meet12);k++)
               {  dim l
               ;  mclv* ct = meet12->cols+k
               ;  mclv* c1mem = c1->cols+k
               ;  mclv* c2mem = NULL
               ;  for (l=0;l<ct->n_ivps;l++)
                  {  ofs c2id = ct->ivps[l].idx
                  ;  double meet_sz = ct->ivps[l].val
                  ;  c2mem = mclxGetVector(c2, c2id, EXIT_ON_FAIL, c2mem)
                  ;  if
                     (  meet_sz < nff_fac * c1mem->n_ivps
                     && meet_sz < nff_fac * c2mem->n_ivps
                     )
                     {  mclv* meet = mcldMeet(c1mem, c2mem, NULL)
                     ;  mclp* tivp = NULL
                     ;  dim m
                     ;  if (i_am_vol)
                        for (m=0;m<meet->n_ivps;m++)
                        {  tivp = mclvGetIvp(nff_scores->cols+0, meet->ivps[m].idx, tivp)
                        ;  tivp->val += 1 / (double) meet->n_ivps
                     ;  }
                        n_volatile += meet->n_ivps
                     ;  if (meet->n_ivps != meet_sz)
                        mcxErr(me, "meet size difference")
                     ;  mclvFree(&meet)
                  ;  }
                     if
                     (  meet_sz < 0.5 * c1mem->n_ivps
                     && meet_sz < 0.5 * c2mem->n_ivps
                     )
                     nff[0] += meet_sz
                  ;  if
                     (  meet_sz < 0.75 * c1mem->n_ivps
                     && meet_sz < 0.75 * c2mem->n_ivps
                     )
                     nff[1] += meet_sz
                  ;  if
                     (  meet_sz < 0.90 * c1mem->n_ivps
                     && meet_sz < 0.90 * c2mem->n_ivps
                     )
                     nff[2] += meet_sz
                  ;  if
                     (  meet_sz < 0.95 * c1mem->n_ivps
                     && meet_sz < 0.95 * c2mem->n_ivps
                     )
                     nff[3] += meet_sz
                  ;  if
                     (  meet_sz < 0.99999 * c1mem->n_ivps
                     && meet_sz < 0.99999 * c2mem->n_ivps
                     )
                     nff[4] += meet_sz
               ;  }
               }
               if (i_am_vol)  /* hacked; refactor later */
               continue
         ;  }

            if (mode_g == DIST_SPLITJOIN)
               clmSJDistance(c1, c2, meet12, meet21, &dist1i, &dist2i)
            ,  fprintf
               (  xfout->fp
               ,  "d=%lu\td1=%lu\td2=%lu\tnn=%ld\tc1=%ld\tc2=%ld\tv=%ld\tn1=%s\tn2=%s\tvol=[%ld,%ld,%ld,%ld,%ld]"
               ,  (ulong) (dist1i + dist2i)
               ,  (ulong) dist1i
               ,  (ulong) dist2i
               ,  (long) N_ROWS(c1)
               ,  (long) N_COLS(c1)
               ,  (long) N_COLS(c2)
               ,  (long) n_volatile
               ,  stptr1->level[i].fname->str
               ,  stptr2->level[j].fname->str
               ,  (long) nff[0]
               ,  (long) nff[1]
               ,  (long) nff[2]
               ,  (long) nff[3]
               ,  (long) nff[4]
               )

         ;  else if (mode_g == DIST_VARINF)
               clmVIDistance(c1, c2, meet12, &dist1d, &dist2d)
            ,  fprintf
               (  xfout->fp
               ,  "d=%.3f\td1=%.3f\td2=%.3f\tnn=%ld\tc1=%ld\tc2=%ld\tn1=%s\tn2=%s"
               ,  dist1d + dist2d
               ,  dist1d
               ,  dist2d
               ,  (long) N_ROWS(c1)
               ,  (long) N_COLS(c1)
               ,  (long) N_COLS(c2)
               ,  stptr1->level[i].fname->str
               ,  stptr2->level[j].fname->str
               )

         ;  else if (mode_g == DIST_MIRKIN)
               clmMKDistance(c1, c2, meet12, &dist1i, &dist2i)
            ,  fprintf
               (  xfout->fp
               ,  "d=%ld\td1=%ld\td2=%ld\tnn=%ld\tc1=%ld\tc2=%ld\tn1=%s\tn2=%s"
               ,  dist1i + dist2i
               ,  dist1i
               ,  dist2i
               ,  (long) N_ROWS(c1)
               ,  (long) N_COLS(c1)
               ,  (long) N_COLS(c2)
               ,  stptr1->level[i].fname->str
               ,  stptr2->level[j].fname->str
               )

         ;  else if (mode_g == INDEX)
            {  mclv* sizes_a  =  mclxColSizes(c1, MCL_VECTOR_SPARSE)
            ;  mclv* sizes_b  =  mclxColSizes(c2, MCL_VECTOR_SPARSE)
            ;  double soba    =  mclv_choose_sum(sizes_a)   /* sum of binomial */
            ;  double sobb    =  mclv_choose_sum(sizes_b)   /* sum of binomial */
            ;  double sobm    =  mclx_choose_sum(meet12)
            ;  double chsn_   =  (N_ROWS(c1) * 0.5 * (N_ROWS(c1) - 1.0))
            ;  double chsn    =  chsn_ ? chsn_ : 0.001   /* crazy, yes */

            ;  double rand    =  (chsn -  soba -  sobb +  2.0 * sobm) / chsn
            ;  double jaccard =  sobm / (soba + sobb - sobm)
            ;  double randc   =     (sobm - (soba * sobb) / chsn)
                                 /  ( 0.5 * (soba + sobb) - (soba * sobb) / chsn)
            ;  fprintf
               (  xfout->fp, "rand=%.5f jaccard=%.5f arand=%.5f n1=%s n2=%s"
               ,  rand, jaccard, randc
               ,  stptr1->level[i].fname->str ,  stptr2->level[j].fname->str
               )
         ;  }

            {  const char* e = getenv("CLMDIST_TAG")
            ;  if (e)
               fprintf(xfout->fp, "\ttag=%s", e)
            ;  fputc('\n', xfout->fp)
         ;  }

            mclxFree(&meet12)
         ;  mclxFree(&meet21)
         ;  if (consecutive_g)
            break
      ;  }
         if (wheel_g)
         break
   ;  }

      if (i_am_vol)
      {  mclxaWrite(nff_scores, xfout, 4, RETURN_ON_FAIL)
   ;  }
      return STATUS_OK
;  }


mcxDispHook* mcxDispHookDist
(  void
)
   {  static mcxDispHook distEntry
   =  {  "dist"
      ,  "dist [options] <cl file>+"
      ,  distOptions
      ,  sizeof(distOptions)/sizeof(mcxOptAnchor) - 1
      ,  distArgHandle
      ,  distInit
      ,  distMain
      ,  0
      ,  -1
      ,  MCX_DISP_MANUAL
      }
   ;  return &distEntry
;  }


mcxDispHook* mcxDispHookVol
(  void
)
   {  static mcxDispHook volEntry
   =  {  "vol"
      ,  "vol [options] <cl file>+"
      ,  volOptions
      ,  sizeof(volOptions)/sizeof(mcxOptAnchor) - 1
      ,  volArgHandle
      ,  volInit
      ,  distMain
      ,  1
      ,  -1
      ,  MCX_DISP_MANUAL
      }
   ;  return &volEntry
;  }


