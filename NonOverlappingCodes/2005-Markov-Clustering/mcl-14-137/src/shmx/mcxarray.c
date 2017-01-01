/*   (C) Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007 Stijn van Dongen
 *   (C) Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/*    TODO

-  { -j -J } should work independently from -t; different
   jobs should be able to have different number of threads.
   So, hierarchical slicing.

-  Extended Jacquard; in(x,y) / ( ||x||^2 + ||y||^2 - in(x,y) )
-  Dice coefficient  2 * in(x,y)  / ( ||x||^2 + ||y||^2 )

-  Kendall Tau
   Fast algorithms for the calculation of Kendall’s Tau, David Christensen.

-  Speed up all vs all in metric case.
      M-tree, "Searching in Metric Spaces", Chavez et al
      "Fast approximate hierarchical clustering using similarity heuristics", Meelis Kull and Jaak Vilo
      "Practical Construction of k-Nearest Neighbor Graphs in Metric Spaces", Rodrigo Paredes et al
   ,  given disjoint thresholded graphs, efficiently compute the thresholded
      cross-network edges.
   ,  given thresholded 0.9 graph (Pearson), efficiently compute the thresholded
      0.8 graph -- note: distances corresponding to 0.9, 0.8 are 0.435 and 0.6.

-  write jobinfo as a matrix (each job one column; threads enumerated).

-  should by_set_size not depend on sparse encoding on data, i.e.
   complementary to embedding? Probable interaction with zero_asna.

-  table reading elsewhere in mcl-edge

http://www.mathworks.com/matlabcentral/fileexchange/15935-computing-pairwise-distances-and-metrics/content/pwmetric/slmetric_pw.m

A
B
da = A\B       (left difference, a)
db = B\A       (right difference, b)
c  = A /\ B    (co-occuring)
ca
cb
e  = A \/ B    (everything)
d  = e \ c     (difference)

sum(a)
sum(a,2)
in(ca, cb)

restrict(a,c)  a restricted to c
restrict
union(a,c,max)
union(a,c,add)
a+1
a*2
a[a>3]   restrict(a, a>3)




Monve, V.; Introduction to Similarity Searching in Chemistry.
www.orgchm.bas.bg/~vmonev/SimSearch.pdf
 fingerprint distances:

      euclidean          D    sqrt(hamming)
      hamming            D    a+b
      meanHamming             (a+b)/(a+b+c+d)
      soergel            D    (a+b)/(a +b +c)
      patternDifference       ab / (a+b+c+d)^2
      variance                (a+b)/4(a+b+c+d)
      size                    (a−b)^2/(a+b+c+d)^2
      shape                   (a+b)/(a+b+c+d) - [(a−b) (a+b+c+d)]^2
  +   jaccard/tanimoto        c / (a+b+c)
      dice                    2c / (a+b)
      mt
      simple
      russelrao
      rodgerstanimoto
  +   cosine                  c / sqrt(AB)
      achiai
      carbo
      baroniurbanibuser       sqrt( cd + c ) / sqrt( cd + a + b + c )
      kulczynski2             0.5 * (c/(a+c) + c/(b+c))
      robust
      hamann
      yule
      pearson                 (cd−ab) / sqrt((a+c)(b+c)(a+d)(b+d))
      mcconnaughey
      stiles
      simpson
      petke
  +   meet                    c
  +   cover              A    c / (a+c)

*/


#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

#include "impala/compose.h"
#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/pval.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "clew/clm.h"
#include "mcl/transform.h"

#include "util/io.h"
#include "util/ting.h"
#include "util/ding.h"
#include "util/err.h"
#include "util/minmax.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/types.h"
#include "util/array.h"

const char* me = "mcxarray";

const char* syntax = "Usage: mcxarray <-data <data-file> | -imx <mcl-file> [options]";


#define ARRAY_PEARSON      (1 <<   0)
#define ARRAY_COSINE       (1 <<   1)
#define ARRAY_SPEARMAN     (1 <<   2)
#define ARRAY_COSINESKEW   (1 <<   3)     /* experimental */
#define ARRAY_SUBSET_MEET  (1 <<   4)
#define ARRAY_SUBSET_DIFF  (1 <<   5)
#define ARRAY_CONTENT      (1 <<   6)
#define ARRAY_DOT          (1 <<   7)
#define ARRAY_FINGERPRINT  (1 <<   8)
#define ARRAY_SINE         (1 <<   9)
#define ARRAY_ARC          (1 <<  10)
#define ARRAY_ACUTE_ARC    (1 <<  11)
#define ARRAY_HSINE        (1 <<  12)
#define ARRAY_HCOSINE      (1 <<  13)
#define ARRAY_MINKOWSKI    (1 <<  14)

#define MODE_ZEROASNA      (1 <<   0)
#define MODE_TRANSPOSE     (1 <<   1)
#define MODE_JOBINFO       (1 <<   2)
#define MODE_RANKTRANSFORM (1 <<   3)
#define MODE_NOWRITE       (1 <<   4)
#define MODE_SPARSE        (1 <<   5)

#define CONTENT_AVERAGE    (1 <<   0)
#define CONTENT_COSINE     (1 <<   1)
#define CONTENT_EUCLID     (1 <<   2)
#define CONTENT_INTERSECT  (1 <<   3)
#define CONTENT_MEDIAN     (1 <<   4)

#define FP_TANIMOTO              0
#define FP_MEET                  1
#define FP_COSINE                2
#define FP_COVER                 3
#define FP_HAMMING               4

#define CONTENT_DEFAULT    (CONTENT_MEDIAN | CONTENT_COSINE | CONTENT_EUCLID | CONTENT_INTERSECT)


  /* Mutual information (Fast Calculation of pairwise mutual information for gene
   * regulatory network reconstruction)
   * Sample average:
   *
   *                           __                       2        2
   *                           \    - 1/(2h^2) ( (xi-xj) + (yi-yj) )
   *     ____                M /_  e
   *  1  \                      j
   * ---  \    log   ----------------------------------------------------------
   *  M   /             __                     2     __             2       2
   *     /___           \    - 1/(2h^2) (xi-xj)      \    - 1/(2h^2) (yi-yj)
   *       i            /_  e                        /_  e           
   *                     j                            j              
   *
   *
   * See also The mutual information: Detecting and evaluating dependencies between variables
   * R Steuer et al, Bioinformatics 18 Suppl 2 2002, S231-S240.
   * It estimates the bandwidth parameter h to be optimal, given certain assumptions on
   * the density being estimated as
   *                               1/(d+4)
   *   h  ~    sigma * ( N / (d+2) )
   *
   *   where N is the dimension of measurements (per object) and d is the
   *   cardinality (number of objects) of the dataset, and sigma the average
   *   marginal standard deviation.
  */




  /* Pearson correlation coefficient:
   *                     __          __   __ 
   *                     \           \    \  
   *                  n  /_ x y   -  /_ x /_ y
   *   -----------------------------------------------------------
   *      ___________________________________________________________
   *     /       __        __ 2                __        __ 2       |
   * \  /  /     \   2     \       \     /     \   2     \       \ 
   *  \/   \   n /_ x   -  /_ x    /  *  \   n /_ y   -  /_ y    / 
  */


static dim n_thread_l = 0;

static dim start_g = 0;
static dim end_g = 0;

static double minkowski = 0.0;

static double g_epsilon        =  1e-6;


static unsigned int support_modalities = 0;


                           /* first mini job is c=0, d=0..N-1 AND c=N-1, d=N-1 (N+2 tasklets)
                            * second is c=1, d=1..N-1 AND c=N-2, d=N-2..N-1    (N+2 tasklets)
                            * so that mini-jobs are the same size.
                            * Each thread computes a batch of mini-jobs.
                            */
struct jobinfo
{  const mclx* tbl  
;  dim n_thread
;  dim n_group
;  dim i_group
;  int by_set_size

;  dim dvd_jobsize         /* derived jobsize */
;  dim dvd_joblo1
;  dim dvd_joblo2
;  dim dvd_jobhi1
;  dim dvd_jobhi2
;  dim dvd_work_size           /* either N_COLS() or SUM(set_size) */

;  dim work_size_seen
;
}  ;


static void ji_init
(  struct jobinfo* ji
,  const mclx* tbl
,  dim n_thread
,  dim n_group
,  dim i_group
,  int by_set_size
)
   {  ji->n_thread   =  n_thread
   ;  ji->n_group    =  n_group
   ;  ji->i_group    =  i_group
   ;  ji->by_set_size=  by_set_size
   ;  ji->work_size_seen = 0
   ;  ji->tbl        =  tbl

                     /* we do note use N_COLS(tbl)^2, as mcxarray may
                      * be used on ragged arrays with very different set sizes
                     */
   ;  if (by_set_size)
      {  dim i
      ;  ji->dvd_work_size = 0
      ;  for (i=0;i<N_COLS(tbl);i++)
         ji->dvd_work_size += tbl->cols[i].n_ivps
   ;  }
      else
      ji->dvd_work_size = N_COLS(tbl)

   ;  ji->dvd_jobsize = (ji->dvd_work_size + 2 * n_thread * n_group - 1) / (2 * n_thread * n_group)
;fprintf(stderr, "---->work size %d job size %d\n", (int) ji->dvd_work_size, (int) ji->dvd_jobsize)

   ;  ji->dvd_joblo1 =  0
   ;  ji->dvd_joblo2 =  0
   ;  ji->dvd_jobhi1 =  N_COLS(tbl)
   ;  ji->dvd_jobhi2 =  N_COLS(tbl)
;  }


                     /* This iterator runs through the full list of jobinfo sets
                      * for all groups and threads, returning 1 in case
                      * the job info suits the current thread and group.
                     */
static int ji_step
(  struct jobinfo* ji
,  dim t
,  dim this_group
)
   {  int bootstrap = ji->dvd_joblo1 == ji->dvd_joblo2
   ;  int accept = t % ji->n_group == ji->i_group
                           /* pick only t that are in i_group */

   ;  if (!bootstrap)
         ji->dvd_joblo1 = ji->dvd_joblo2        /* new offset for the low range */
      ,  ji->dvd_jobhi2 = ji->dvd_jobhi1        /* new top for the high range */

   ;  if (ji->by_set_size)
      {  dim work_size_toreach = (t+1) * 2 * ji->dvd_jobsize
      ;  dim i = 0
      ;  while
         (  ji->work_size_seen < work_size_toreach
         && ji->dvd_joblo2 < ji->dvd_jobhi1
         )
         {  if (!(i & 1))
            ji->work_size_seen += ji->tbl->cols[(ji->dvd_joblo2)++].n_ivps
         ;  else
            ji->work_size_seen += ji->tbl->cols[--(ji->dvd_jobhi1)].n_ivps
         ;  i++
      ;  }
;if(0)fprintf(stdout, "%c %d %d %d\n", accept ? (int) ('0' + this_group) : '-', (int) ji->work_size_seen, (int) ji->dvd_joblo1, (int) ji->dvd_joblo2)
;if(0)fprintf(stdout, "%c %d %d %d\n", accept ? (int) ('0' + this_group) : '-', (int) ji->work_size_seen, (int) ji->dvd_jobhi1, (int) ji->dvd_jobhi2)
   ;  }
      else
      {  ji->dvd_joblo2 += ji->dvd_jobsize

      ;  if (ji->dvd_jobhi1 >= ji->dvd_jobsize)
         ji->dvd_jobhi1 -= ji->dvd_jobsize
   ;  }

      return accept ?  1 :  0
;  }


double mclvChebyshev
(  const mclVector*        lft
,  const mclVector*        rgt
)
   {  double norm = 0.0
   ;  mclp
         *ivp1    = lft->ivps
      ,  *ivp2    = rgt->ivps
      ,  *ivp1max = ivp1 + lft->n_ivps
      ,  *ivp2max = ivp2 +rgt->n_ivps

   ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
      {  if (ivp1->idx < ivp2->idx)
         ivp1++
      ;  else if (ivp1->idx > ivp2->idx)
         ivp2++
      ;  else
         {  double d = fabs((ivp1++)->val - (ivp2++)->val)
         ;  if (d > norm)
            norm = d
      ;  }
      }
      while (ivp1 < ivp1max)
      {  double d = fabs(ivp1++->val)
      ;  if (d > norm)
         norm = d
   ;  }
      while (ivp2 < ivp2max)
      {  double d = fabs(ivp2++->val)
      ;  if (d > norm)
         norm = d
   ;  }
      return norm
;  }


double mclvMinkowski
(  const mclVector*        lft
,  const mclVector*        rgt
,  double p
)
   {  double norm = 0.0
   ;  mclp
         *ivp1    = lft->ivps
      ,  *ivp2    = rgt->ivps
      ,  *ivp1max = ivp1 + lft->n_ivps
      ,  *ivp2max = ivp2 +rgt->n_ivps

   ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
      {  if (ivp1->idx < ivp2->idx)
         ivp1++
      ;  else if (ivp1->idx > ivp2->idx)
         ivp2++
      ;  else
         norm += pow(fabs((ivp1++)->val - (ivp2++)->val), p)
   ;  }
      while (ivp1 < ivp1max)
      norm += pow(fabs((ivp1++)->val), p)
   ;  while (ivp2 < ivp2max)
      norm += pow(fabs((ivp2++)->val), p)

   ;  return pow(norm, 1/p)
;  }


static double mclv_inner_minkowski
(  const mclv* a
,  const mclv* b
,  dim N
)
   {  dim j
   ;  double norm = 0.0

   ;  if (a->n_ivps < N || b->n_ivps < N)
      return minkowski ? mclvMinkowski(a, b, minkowski) : mclvChebyshev(a, b)

   ;  if (minkowski)
      {  for (j=0;j<a->n_ivps;j++)
         norm += pow(fabs(a->ivps[j].val - b->ivps[j].val), minkowski)
      ;  return pow(norm, 1/minkowski)
   ;  }

      else
      {  for (j=0;j<a->n_ivps;j++)
         {  double d = fabs(a->ivps[j].val - b->ivps[j].val)
         ;  if (d > norm)
            norm = d
      ;  }
         return norm
   ;  }

      return 0.0
;  }


static double mclv_inner_dot
(  const mclv* a
,  const mclv* b
,  dim N
)
   {  dim j
   ;  double ip = 0.0

   ;  if (a->n_ivps < N || b->n_ivps < N)
      return mclvIn(a, b)

   ;  for (j=0;j<a->n_ivps;j++)
      ip += a->ivps[j].val * b->ivps[j].val
   ;  return ip
;  }


double pearson
(  const mclv* a
,  const mclv* b
,  dim n
)
   {  double suma = mclvSum(a)
   ;  double sumb = mclvSum(b)
   ;  double sumasq = mclvPowSum(a, 2.0)
   ;  double sumbsq = mclvPowSum(b, 2.0)

   ;  double nom = sqrt( (n*sumasq - suma*suma) * (n*sumbsq - sumb*sumb) )
   ;  double num = n * mclvIn(a, b) - suma * sumb
   ;  return nom ? num / nom : 0.0
;  }


typedef struct rank_unit
{  long     index
;  double   ord
;  double   value
;
}  rank_unit   ;


void*  rank_unit_init
(  void*   ruv
)
   {  rank_unit* ru =   (rank_unit*) ruv
   ;  ru->index     =   -1
   ;  ru->value     =   -1
   ;  ru->ord       =   -1.0
   ;  return ru
;  }


int rank_unit_cmp_index
(  const void* rua
,  const void* rub
)
   {  long a = ((rank_unit*) rua)->index
   ;  long b = ((rank_unit*) rub)->index
   ;  return a < b ? -1 : a > b ? 1 : 0
;  }


int rank_unit_cmp_value
(  const void* rua
,  const void* rub
)
   {  double a = ((rank_unit*) rua)->value
   ;  double b = ((rank_unit*) rub)->value
   ;  return a < b ? -1 : a > b ? 1 : 0
;  }


enum
{  MY_OPT_DATA
,  MY_OPT_TABLE
,  MY_OPT_CUTOFF
,  MY_OPT_O
,  MY_OPT_TEXTTABLE
,  MY_OPT_WRITEBINARY
               ,  MY_OPT_TAB

,  MY_OPT_RSKIP = MY_OPT_TAB + 2
,  MY_OPT_CSKIP
               ,  MY_OPT_L

,  MY_OPT_PEARSON = MY_OPT_L + 2
,  MY_OPT_SPEARMAN
,  MY_OPT_COSINE
,  MY_OPT_SINE
,  MY_OPT_HCOSINE
,  MY_OPT_HSINE
,  MY_OPT_ARC
,  MY_OPT_ACUTE_ARC
,  MY_OPT_ARC_NORM
,  MY_OPT_ACUTE_ARC_NORM
,  MY_OPT_EUCLID
,  MY_OPT_TAXI
,  MY_OPT_L00
,  MY_OPT_MINKOWSKI
,  MY_OPT_DOT
,  MY_OPT_SUBSET_MEET
,  MY_OPT_SUBSET_DIFF
,  MY_OPT_CONTENT
,  MY_OPT_CONTENTX
,  MY_OPT_NOTES
,  MY_OPT_FINGERPRINT
,  MY_OPT_SPARSE
               ,  MY_OPT_COSINE_SKEW

,  MY_OPT_T    =  MY_OPT_COSINE_SKEW + 2
,  MY_OPT_NJOBS
,  MY_OPT_JOBID
,  MY_OPT_JI
,  MY_OPT_START
,  MY_OPT_END

,  MY_OPT_TP   =  MY_OPT_END + 2
,  MY_OPT_TRANSFORM
,  MY_OPT_TABLETRANSFORM
,  MY_OPT_DIGITS
,  MY_OPT_SEQL
,  MY_OPT_SEQR
,  MY_OPT_ZEROASNA
,  MY_OPT_WRITE_TABLE
,  MY_OPT_WRITE_NA
,  MY_OPT_LIMIT_ROWS
,  MY_OPT_NORMALISE
,  MY_OPT_RANKTRANSFORM
,  MY_OPT_NOPROGRESS
,  MY_OPT_NW
,  MY_OPT_HELP
,  MY_OPT_VERSION
,  MY_OPT_AMOIXA
}  ;


mcxOptAnchor options[]
=
{
   {  "-h"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_HELP
   ,  NULL
   ,  "print this help"
   }
,  {  "--help"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_HELP
   ,  NULL
   ,  "print this help"
   }
,  {  "--version"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_VERSION
   ,  NULL
   ,  "print version information"
   }
,  {  "--amoixa"
   ,  MCX_OPT_INFO | MCX_OPT_HIDDEN
   ,  MY_OPT_AMOIXA
   ,  NULL
   ,  ">o<"
   }
,  {  "-t"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_T
   ,  "<int>"
   ,  "number of threads to use"
   }
,  {  "-J"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NJOBS
   ,  "<int>"
   ,  "number of compute jobs overall"
   }
,  {  "-j"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_JOBID
   ,  "<int>"
   ,  "index of this compute job"
   }
,  {  "--job-info"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_JI
   ,  NULL
   ,  "print node ids and exit"
   }
,  {  "-start"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_START
   ,  "<int>"
   ,  "start index"
   }
,  {  "-end"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_END
   ,  "<int>"
   ,  "end index"
   }
,  {  "--transpose-data"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TP
   ,  NULL
   ,  "work with the transposed data matrix"
   }
,  {  "-skipc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CSKIP
   ,  "<num>"
   ,  "skip this many columns"
   }
,  {  "-seql"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_SEQL
   ,  "<fname>"
   ,  "file with start positions"
   }
,  {  "-seqr"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_SEQR
   ,  "<fname>"
   ,  "file with end positions"
   }
,  {  "-V"
   ,   MCX_OPT_HIDDEN
   ,  MY_OPT_NOPROGRESS
   ,  NULL
   ,  "omit progress bar"
   }
,  {  "-skipr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RSKIP
   ,  "<num>"
   ,  "skip this many rows"
   }
,  {  "--rank-transform"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RANKTRANSFORM
   ,  NULL
   ,  "rank transform the data"
   }
,  {  "-n"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NORMALISE
   ,  "{P,S} x {z,m}"
   ,  "normalise P(rimary) or S(econdary) on z-scores (z) or mean (m)"
   }
,  {  "--pearson"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_PEARSON
   ,  NULL
   ,  "compute edge weight as Pearson correlation score (default)"
   }
,  {  "--spearman"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SPEARMAN
   ,  NULL
   ,  "compute edge weight as Spearman rank correlation score"
   }
,  {  "--zero-as-na"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_ZEROASNA
   ,  NULL
   ,  "compute correlation only where both values are not zero"
   }
,  {  "--cosine"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_COSINE
   ,  NULL
   ,  "compute edge weight as cosine"
   }
,  {  "--sine"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SINE
   ,  NULL
   ,  "compute edge weight as sine"
   }
,  {  "--slow-cosine"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_HCOSINE
   ,  NULL
   ,  "compute edge weight as cosine"
   }
,  {  "--slow-sine"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_HSINE
   ,  NULL
   ,  "compute edge weight as sine"
   }
,  {  "--angle"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_ARC
   ,  NULL
   ,  "compute edge weight as arc length"
   }
,  {  "--acute-angle"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_ACUTE_ARC
   ,  NULL
   ,  "compute edge weight as acute arc length"
   }
,  {  "--angle-norm"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_ARC_NORM
   ,  NULL
   ,  "compute edge weight as arc length, normalised by pi"
   }
,  {  "--acute-angle-norm"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_ACUTE_ARC_NORM
   ,  NULL
   ,  "compute edge weight as acute arc length, normalised by pi/2"
   }
,  {  "--dot"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DOT
   ,  NULL
   ,  "compute edge weight as dot product"
   }
,  {  "--euclid"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_EUCLID
   ,  NULL
   ,  "compute edge weight as Euclidean distance"
   }
,  {  "--taxi"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TAXI
   ,  NULL
   ,  "compute edge weight as taxi (city block) distance"
   }
,  {  "--max"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_L00
   ,  NULL
   ,  "compute edge weight as max (aka L-oo or Chebyshev) distance"
   }
,  {  "-minkowski"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MINKOWSKI
   ,  "<val>"
   ,  "compute edge weight as minkowski distance with power p = <val>"
   }
,  {  "--cosine-skew"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_COSINE_SKEW
   ,  NULL
   ,  "compute arc weight SQRT(<self * other>) / || self ||"
   }
,  {  "--content"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_CONTENT
   ,  NULL
   ,  "compute fantastic mr formula"
   }
,  {  "--no-write"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_NW
   ,  NULL
   ,  "exit after computation of correlations"
   }
,  {  "-content"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_CONTENTX
   ,  "(v)verage (c)osine (e)uclid (i)ntersect (m)edian"
   ,  "compute fantastic mr formula with parts specified"
   }
,  {  "--notes"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_NOTES
   ,  NULL
   ,  "explain fantastic mr formula"
   }
,  {  "--sparse"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SPARSE
   ,  NULL
   ,  "do not embed zero elements; ignore zero elements"
   }
,  {  "-fp"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_FINGERPRINT
   ,  "hamming|meet|tanimoto|cosine|cover"
   ,  "a+b, c, c/(a+b+c), c/sqrt((a+c)*(b+c)), c/(a+c)"
   }
,  {  "--subset"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_SUBSET_MEET
   ,  NULL
   ,  "compute arc weight |self /\\ other>| / |self|"
   }
,  {  "--subset-diff"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_SUBSET_DIFF
   ,  NULL
   ,  "compute arc weight |self \\ other>| / |self|"
   }
,  {  "--text-table"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TEXTTABLE
   ,  NULL
   ,  "write output in full text table format with tab-separated values (N-squared entries)"
   }
,  {  "--write-binary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITEBINARY
   ,  NULL
   ,  "write in binary format (use with low -co and subsequent mcx q --vary-threshold)"
   }
,  {  "-data"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DATA
   ,  "<fname>"
   ,  "data file name"
   }
,  {  "-write-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "write labels to tab file"
   }
,  {  "-l"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_L
   ,  "<int>"
   ,  "column (or row, with --transpose) containing labels (default 1)"
   }
,  {  "-write-data"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_WRITE_TABLE
   ,  "<fname>"
   ,  "write table matrix to file"
   }
,  {  "-write-na"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_WRITE_NA
   ,  "<fname>"
   ,  "write na matrix to file"
   }
,  {  "-test-rows"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_LIMIT_ROWS
   ,  "<num>"
   ,  "only do this many data rows"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TABLE
   ,  "<fname>"
   ,  "matrix file name"
   }
,  {  "-co"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CUTOFF
   ,  "<num>"
   ,  "only output values val with |val| >= <num>"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_O
   ,  "<fname>"
   ,  "write to file fname"
   }
,  {  "-table-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TABLETRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to table values"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to result matrix values"
   }
,  {  "-digits"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DIGITS
   ,  "<int>"
   ,  "precision to use in interchange format"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


static mclTab* tab_g = NULL;
static mcxbool sym_g = TRUE;
static mcxbits contentx_g = 0;
static mcxenum fingerprint_g = 0;
static mcxbool progress_g = TRUE;

static const char* retry_g = "out.mcxarray";


      /* user rows will be columns in mcl native layout.
       * naming below is confusing;  clean up naming sometime.
      */
static mclx* read_data
(  mcxIO* xfin
,  mcxIO* xftab
,  dim nrows_max
,  unsigned skipr
,  unsigned skipc
,  unsigned labelidx
,  mcxbits  bits
,  mclx**   mxnapp
)
   {  mcxTing* line = mcxTingEmpty(NULL, 1000)
   ;  dim  N_cols =  0
   ;  int  n_rows =  0
   ;  int  N_rows =  100

   ;  mclv* cols  =  mcxNAlloc(N_rows, sizeof(mclVector), mclvInit_v, EXIT_ON_FAIL)
   ;  mclv* colsna=  mcxNAlloc(N_rows, sizeof(mclVector), mclvInit_v, EXIT_ON_FAIL)

   ;  mclv* scratch = mclvCanonical(NULL, 100, 1.0)
   ;  mclv* scratchna = mclvCanonical(NULL, 100, 1.0)

   ;  mclx* mx    =  mcxAlloc(sizeof mx[0], EXIT_ON_FAIL)
   ;  mclx* mxna  =  mcxAlloc(sizeof mxna[0], EXIT_ON_FAIL)

   ;  mcxHash* index = xftab ? mcxHashNew(100, mcxStrHash, mcxStrCmp) : NULL       /* user rows */
   ;  long linect = 0
   ;  unsigned skiprr = skipr

   ;  while (STATUS_OK == mcxIOreadLine(xfin, line, MCX_READLINE_CHOMP))
      {  const char* p = line->str
      ;  const char* z = p + line->len
      ;  dim n_cols = 0, n_na = 0
      ;  double val = 0.0
      ;  unsigned skipcc = skipc

      ;  linect++

      ;  if (skiprr > 0)
         {  if ((support_modalities & MODE_TRANSPOSE) && skipr+1 - skiprr == labelidx)        /* this is the line with labels */
            {  while (p < z)
               {  const char* o = p
               ;  p = strchr(p, '\t')

               ;  if (!p)
                  p = z

               ;  if (skipcc == 0)              /* skipped all we needed to */
                  {  N_cols++
                  ;  if (xftab)
                     {  char* label = mcxStrNDup(o, (dim) (p-o))
                     ;  mcxKV* kv = mcxHashSearch(label, index, MCX_DATUM_INSERT)
                     ;  if (kv->key != label)
                        mcxDie(1, me, "column label <%s> occurs more than once", label)
                     ;  fprintf(xftab->fp, "%d\t%s\n", (int) (N_cols-1), label)
                  ;  }
                  }
                  else
                  skipcc--
               ;  p++
            ;  }
            }
            skiprr--
         ;  continue
      ;  }

         {  mcxbool just_after_tab = TRUE   /*  first time around fake it */  
         ;  while (just_after_tab)
            {  mcxbool have_na = TRUE
            ;  const char* o = p
            ;  char* t = strchr(p, '\t')
            ;  just_after_tab = t ? TRUE : FALSE      /* this is for the next iteration */

            ;  p = t ? t : z

            ;  if (skipcc > 0)
               {  if (!(support_modalities & MODE_TRANSPOSE) && xftab && (skipc+1-skipcc) == labelidx)
                  {  char* label = mcxStrNDup(o, (dim) (p-o))
                  ;  mcxKV* kv = mcxHashSearch(label, index, MCX_DATUM_INSERT)
                  ;  if (kv->key != label)
                     mcxDie(1, me, "row label <%s> occurs more than once", label)
                  ;  fprintf(xftab->fp, "%d\t%s\n", n_rows, label)
               ;  }
                  skipcc--
               ;  p++
               ;  continue
            ;  }

               if (p - o == 0)
               val = 0.0
            ;  else if (p - o == 3 && (!strncasecmp(o, "NaN", 3) || !strncasecmp(o, "inf", 3)))
               val = 0.0
            ;  else if (p - o == 2 && !strncasecmp(o, "NA", 2))
               val = 0.0
            ;  else if (1 == sscanf(o, "%lf", &val))
               have_na = FALSE
            ;  else
               {  mcxDie
                  (  1
                  ,  me
                  ,  "failed to parse column %lu at line %lu offset %lu (---->%.8s<----)"
                  ,  (ulong) (n_cols+1+skipr)
                  ,  (ulong) linect
                  ,  (ulong) (o - line->str + 1)
                  ,  o
                  )
               ;  n_cols = 0
            ;  }

               if (n_cols >= scratch->n_ivps)
               mclvCanonicalExtend(scratch, scratch->n_ivps * 1.44, 0.0)
            ;  scratch->ivps[n_cols].val = val

            ;  if (have_na)
               {  if (n_na >= scratchna->n_ivps)
                  mclvResize(scratchna, n_na * 1.44)
               ;  scratchna->ivps[n_na].val = 1.0
               ;  scratchna->ivps[n_na].idx = n_cols
;if(0)fprintf(stderr, "missing value column %d\n", (int) n_cols)
               ;  n_na++
            ;  }

               n_cols++
            ;  p++
         ;  }
         }

         if (!n_cols && !N_cols)
         mcxDie(1, "mcxarray", "nothing read at line %lu", (ulong) linect)
      ;  else if (!N_cols)
         N_cols = n_cols
      ;  else if (n_cols != N_cols)
         mcxDie
         (  1
         ,  "mcxarray"
         ,  "different column count at line %lu: %lu (expecting %lu)"
         ,  (ulong) linect
         ,  (ulong) N_cols
         ,  (ulong) n_cols
         )

      ;  if (n_rows == N_rows)
         {  N_rows *= 1.44
         ;  cols
            =  mcxNRealloc
               (cols, N_rows, n_rows, sizeof(mclVector), mclvInit_v, EXIT_ON_FAIL)
         ;  colsna
            =  mcxNRealloc
               (colsna, N_rows, n_rows, sizeof(mclVector), mclvInit_v, EXIT_ON_FAIL)
      ;  }

         mclvFromIvps(cols+n_rows, scratch->ivps, n_cols)
      ;  cols[n_rows].vid = n_rows

      ;  mclvFromIvps(colsna+n_rows, scratchna->ivps, n_na)
      ;  colsna[n_rows].vid = n_rows

      ;  n_rows++
      ;  if (nrows_max && n_rows >= nrows_max)
         break
   ;  }

      if (!n_rows)
      mcxErr(me, "no rows read")

   ;  if (xftab)
      {  if (!strcmp(xftab->fn->str, "-"))
         mcxErr(me, "cannot read back tab, warnings will be as offsets")
      ;  else
            mcxIOclose(xftab)
         ,  mcxIOrenew(xftab, NULL, "r")
         ,  tab_g = mclTabRead(xftab, NULL, EXIT_ON_FAIL)
   ;  }

      if (xftab)
      mcxIOclose(xftab)

   ;  mx->dom_cols = mclvCanonical(NULL, n_rows, 1.0)
   ;  mx->dom_rows = mclvCanonical(NULL, N_cols, 1.0)
   ;  mx->cols
      =  mcxNRealloc(cols, n_rows, N_rows, sizeof(mclVector), NULL, EXIT_ON_FAIL)

   ;  mxna->dom_cols = mclvCanonical(NULL, n_rows, 1.0)
   ;  mxna->dom_rows = mclvCanonical(NULL, N_cols, 1.0)
   ;  mxna->cols
      =  mcxNRealloc(colsna, n_rows, N_rows, sizeof(mclVector), NULL, EXIT_ON_FAIL)

   ;  mcxTingFree(&line)
   ;  mclvFree(&scratch)
   ;  mclvFree(&scratchna)

   ;  *mxnapp = mxna
   ;  mcxTell(me, "read table with %d rows and %d columns", (int) N_cols, (int) n_rows)
                           /* ^ note mcl stores rows column-wise */
   ;  return mx
;  }


double mydiv
(  pval a
,  pval b
)
   {  return b ? a / b : 0.0
;  }



static mclx* mxseql = NULL;
static mclx* mxseqr = NULL;



static double ivp_get_double
(  const void* v
)
   {  return ((mclp*) v)->val
;  }


static double get_array_content_score
(  const mclx* tbl
,  dim c
,  dim d
,  mclv* meet_c
)
   {  mclv* vecc, *vecd, *meet_d
   ;  double euclid = 1.0, meet_fraction = 1.0, score , sum_meet_c, sum_meet_d
   ;  double iqrc, iqrd
   ;  double reduction_c = 1.0, reduction_d = 1.0
   ;  double median = 1.0, medianc = 1.0, mediand = 1.0
   ;  double ip = 1.0, cd = 1.0, csn = 1.0
   ;  double mean = 1.0, meanc = 1.0, meand = 1.0

   ;  vecc = mclvClone(tbl->cols+c)
   ;  vecd = mclvClone(tbl->cols+d)
   ;  meet_d = mcldMeet(vecd, meet_c, NULL)

            /* the only purpose is here is to reweight the scores in meet_c */
   ;  sum_meet_c = mclvSum(meet_c)
   ;  sum_meet_d = mclvSum(meet_d)

   ;  if (!sum_meet_c || !sum_meet_d)
      return 0.0

   ;  if (mxseql && mxseqr)
      {  const mclv* c_start = mxseql->cols+c
      ;  const mclv* c_end   = mxseqr->cols+c
      ;  const mclv* d_start = mxseql->cols+d
      ;  const mclv* d_end   = mxseqr->cols+d

      ;  mclv* width_c =  mclvBinary(c_end, c_start, NULL, fltSubtract)
      ;  mclv* width_d =  mclvBinary(d_end, d_start, NULL, fltSubtract)

      ;  mclv* rmin   = mclvBinary(c_end, d_end, NULL, fltMin)
      ;  mclv* lmax   = mclvBinary(c_start, d_start, NULL, fltMax)
      ;  mclv* delta  = mclvBinary(rmin, lmax, NULL, fltSubtract)
      ;  mclv* weight_c, *weight_d

      ;  mclvSelectGqBar(delta, 0.0)
      ;  weight_c = mclvBinary(delta, width_c, NULL, mydiv)
      ;  weight_d = mclvBinary(delta, width_d, NULL, mydiv)

      ;  mclvBinary(meet_c, weight_c, meet_c, fltMultiply)
      ;  mclvBinary(meet_d, weight_d, meet_d, fltMultiply)

;if(0&&c!=d)
mclvaDump(width_c,stdout,5,"\n",0),mclvaDump(width_d,stdout,5,"\n",0)

      ;  mclvFree(&width_c)
      ;  mclvFree(&width_d)
      ;  mclvFree(&rmin)
      ;  mclvFree(&lmax)
      ;  mclvFree(&delta)
      ;  mclvFree(&weight_c)
      ;  mclvFree(&weight_d)

      ;  reduction_c = mclvSum(meet_c) / sum_meet_c
      ;  reduction_d = mclvSum(meet_d) / sum_meet_d
   ;  }

      score = 1.0

               /* A and B are vectors; denote the indices on which both
                * A and B are nonzero by A/\\B
                * A* denotes the subset of entries of A restricted to A/\\B
                * B* denotes the subset of entries of B restricted to A/\\B
                * Then:
                * median:  min(median(A), median(B))
                * average: min(avg(A*), avg(B*))
                * cosine:  cosine(A*, B*)
                * euclid:  sqrt(sumsq(A*) / sumsq(A))
                * isect:   #A* / #A
                *
                *         |||||
                *       |||||||||
                *    ||||||||||||||||
                * ||||||||||||||||||||||||||||||||||||||
                *
                *               |||||
                *             |||||||||
                *          ||||||||||||||||
                *     ||||||||||||||||||||||||||||||||||||||||
                *
                *     ----------------------------------
               */
   ;  if (contentx_g & CONTENT_MEDIAN)
      {  mclvSortDescVal(vecc)
      ;  mclvSortDescVal(vecd)
      ;  medianc = mcxMedian(vecc->ivps, vecc->n_ivps, sizeof vecc->ivps[0], ivp_get_double, &iqrc)
      ;  mediand = mcxMedian(vecd->ivps, vecd->n_ivps, sizeof vecd->ivps[0], ivp_get_double, &iqrd)
      ;  median = MCX_MIN(medianc, mediand)
      ;  score *= median
   ;  }
      else if (contentx_g & CONTENT_AVERAGE)
      {  meanc = meet_c->n_ivps ? mclvSum(meet_c) / meet_c->n_ivps : 0.0
      ;  meand = meet_d->n_ivps ? mclvSum(meet_d) / meet_d->n_ivps : 0.0
      ;  mean  = MCX_MIN(meanc, meand)
      ;  score *= mean
   ;  }

      if (contentx_g & CONTENT_COSINE)
      {  ip = mclvIn(meet_c, meet_d)
      ;  cd = sqrt(mclvPowSum(meet_c, 2.0) * mclvPowSum(meet_d, 2.0))
      ;  csn = cd ? ip / cd : 0.0
      ;  score *= csn
   ;  }

      if (contentx_g & CONTENT_EUCLID)
      {  euclid = reduction_c ? sqrt(mclvPowSum(meet_c, 2.0) / mclvPowSum(vecc, 2.0)) : 0.0
      ;  score *= euclid
   ;  }

      if (contentx_g & CONTENT_INTERSECT)
      {  meet_fraction = meet_c->n_ivps * 1.0 / vecc->n_ivps
      ;  score *= meet_fraction
   ;  }

      if (0)
      fprintf
      (  stdout
      ,  "%10d%10d%10d%10d%10d%10g%10g%10g%10g%10g%10g%10g%15g\n"
      ,  (int) c
      ,  (int) d
      ,  (int) (vecc->n_ivps - meet_c->n_ivps)
      ,  (int) (vecd->n_ivps - meet_d->n_ivps)
      ,  (int) meet_c->n_ivps
      ,  score
      ,  mean
      ,  csn
      ,  euclid
      ,  meet_fraction
      ,  reduction_c
      ,  reduction_d
      ,  median
      )


   ;  mclvFree(&meet_d)
   ;  mclvFree(&vecc)
   ;  mclvFree(&vecd)

   ;  return score
;  }




#if 0
         {  mclv* meet_d = mcldMeet2(vecd, meet_c, NULL)
         ;  double ip   =  mclvIn(meet_c, meet_d)
         ;  double cd   =  sqrt(mclvPowSum(meet_c, 2.0) * mclvPowSum(meet_d, 2.0))
         ;  double csn  =  cd ? ip / cd : 0.0
         ;  double meanc = mclvSum(meet_c) / meet_c->n_ivps
         ;  double meand = mclvSum(meet_d) / meet_d->n_ivps
         ;  double mean =  MCX_MIN(meanc, meand)

         ;  double euclid =   0
                           ?  1.0
                           :  (  mean
                              ?  sqrt(mclvPowSum(meet_c, 2.0) / mclvPowSum(vecc, 2.0))
                              :  0.0
                              )
         ;  score       =  mean * csn * euclid  * (meet_c->n_ivps * 1.0 / vecc->n_ivps)

   ;if (0 && score)
   fprintf(stdout, "c=%lu d=%lu meet=%d mean=%5g csn=%5g euclid=%5g score=%5g\n", (ulong) c, (ulong) d, (int) meet_c->n_ivps, mean, csn, euclid, score)
         ;  nom = 1                    /* nonsensical; document */
         ;  if (!vecd->n_ivps)
            offending = d              /* nonsensical; document */
         ;  mclvFree(&meet_d)
      ;  }
#endif


     /* Pearson correlation coefficient:
      *                     __          __   __ 
      *                     \           \    \  
      *                  n  /_ x y   -  /_ x /_ y
      *   -----------------------------------------------------------
      *      ___________________________________________________________
      *     /       __        __ 2                __        __ 2       |
      * \  /  /     \   2     \       \     /     \   2     \       \ 
      *  \/   \   n /_ x   -  /_ x    /  *  \   n /_ y   -  /_ y    / 
     */

static dim get_correlation
(  const mclx* tbl
,  const mclx* mxna
,  dim c
,  dim d
,  mclv* Nssqs
,  mclv* sums
,  double* scorep
,  double* nomp
,  dim* offendingp
,  mcxbits bits
)
   {  double N  = MCX_MAX(N_ROWS(tbl), 1)      /* fixme; bit odd */
   ;  double nom = 1.0, score = 0
   ;  dim offending = c
   ;  dim n_reduced = 0
   ;  mclv* vecc = tbl->cols+c
   ;  mclv* vecd = tbl->cols+d

   ;  if (bits & ARRAY_COSINE)
      {  mcxbool reduced= mxna->cols[c].n_ivps > 0 || mxna->cols[d].n_ivps > 0
      ;  double ip      =  mclv_inner_dot(vecc, vecd, N)
      ;  double nomleft = Nssqs->ivps[c].val
      ;  double nomright= Nssqs->ivps[d].val

      ;  if (reduced)
         {  mclv* merge = mcldMerge(mxna->cols+c, mxna->cols+d, NULL)
         ;  mclv* veccx = mcldMinus(vecc, merge, NULL)
         ;  mclv* vecdx = mcldMinus(vecd, merge, NULL)

         ;  nomleft     = N * mclvPowSum(veccx, 2.0)
         ;  nomright    = N * mclvPowSum(vecdx, 2.0)

         ;  mclvFree(&merge)
         ;  mclvFree(&veccx)
         ;  mclvFree(&vecdx)
      ;  }

         nom         =  sqrt(nomleft * nomright) / N
      ;  score       =  nom ? ip / nom : 0.0
      ;  offending   =  nomleft ? d : c
      ;  if (bits & ARRAY_SINE)
         score = sqrt(1.0 - score * score)
      ;  else if (bits & ARRAY_HSINE)
         score = sqrt(0.5 - 0.5 * score)
      ;  else if (bits & ARRAY_HCOSINE)
         score = sqrt(0.5 + 0.5 * score)
      ;  else if (bits & ARRAY_ARC)
         {  if (fabs(score) > 1.0)
            score = 1.0
         ;  score = acos(score)
         ;  if ((bits & ARRAY_ACUTE_ARC) && score > acos(0))
            score = acos(-1.0) - score
      ;  }
      }

      else if (bits & ARRAY_DOT)
      score = mclv_inner_dot(vecc, vecd, N)

   ;  else if (bits & ARRAY_MINKOWSKI)
      score = mclv_inner_minkowski(vecc, vecd, N)

   ;  else if (bits & ARRAY_COSINESKEW)
      {  double ip   =  mclv_inner_dot(vecc, vecd, N)
      ;  nom         =  sqrt(Nssqs->ivps[c].val / N)
      ;  score       =  nom ? sqrt(ip > 0 ? ip : 0) / nom : 0.0
      ;  offending   =  c
   ;  }

      else if (bits & ARRAY_CONTENT)
      {  mclv* meet_c = mcldMeet2(vecc, vecd, NULL)
      ;  if
         (  3 * meet_c->n_ivps >= vecc->n_ivps
         || 3 * meet_c->n_ivps >= vecd->n_ivps
         )
         score = get_array_content_score(tbl, c, d, meet_c)
      ;  nom = 1
      ;  mclvFree(&meet_c)
   ;  }

      else if (bits & ARRAY_FINGERPRINT)
      {  dim na = 0, nb = 0, nc = 0
      ;  mcldCountParts(vecc, vecd, &na, &nc, &nb)
      ;  if (nc)
         switch(fingerprint_g)
         {  case FP_TANIMOTO: score = nc * 1.0 / (na + nb + nc); break
         ;  case FP_COSINE: score = nc * 1.0 / sqrt((na + nc) * 1.0 * (nb + nc)); break
         ;  case FP_MEET: score = nc; break
         ;  case FP_COVER: score = nc * 1.0 / (na + nc); break
         ;  case FP_HAMMING: score = na + nb; break
      ;  }
      }

      else if (bits & (ARRAY_SUBSET_MEET | ARRAY_SUBSET_DIFF))
      {  dim n_meet = 0, n_ldiff = 0
      ;  double num  =  0.0
      ;  mcldCountParts(vecc, vecd, &n_ldiff, &n_meet, NULL)
      ;  if (n_meet)
         num = (bits & ARRAY_SUBSET_MEET) ? n_meet : n_ldiff
      ;  nom         =  vecc->n_ivps
      ;  score       =  nom ? num * 1.0 / nom : 0.0
      ;  offending   =  c
   ;  }

      else if (bits & (ARRAY_PEARSON | ARRAY_SPEARMAN))
      {  mcxbool reduced = mxna->cols[c].n_ivps > 0 || mxna->cols[d].n_ivps > 0
      ;  double s1      =  sums->ivps[c].val
      ;  double Nsq1    =  Nssqs->ivps[c].val
      ;  double nomleft =  sqrt(Nsq1 - s1 * s1)
      ;  double ip      =  mclv_inner_dot(vecc, vecd, N)
      ;  double nomleftx

      ;  if (reduced)
         {  mclv* merge = mcldMerge(mxna->cols+c, mxna->cols+d, NULL)
         ;  mclv* veccx = mcldMinus(vecc, merge, NULL)
         ;  mclv* vecdx = mcldMinus(vecd, merge, NULL)

         ;  double s1x  =  mclvSum(veccx)
         ;  double Nsq1x=  N * mclvPowSum(veccx, 2.0)       /* fixme: still use this N ? */

         ;  double s2x  = mclvSum(vecdx)
         ;  double Nsq2x= N * mclvPowSum(vecdx, 2.0)

         ;  n_reduced++

         ;  nomleftx =  sqrt(Nsq1x - s1x * s1x)
         ;  nom      =  nomleftx * sqrt(Nsq2x - s2x*s2x)
         ;  score    =  nom ? ((N*ip - s1x*s2x) / nom) : 0.0
         ;  offending=  nomleftx ? d : c              /* prepare in case !nom */

;if(0)fprintf(stderr, "vec %d have %d vec %d have %d nom %.2f", (int) c, (int) veccx->n_ivps,  (int) d, (int) vecdx->n_ivps, nom)
;if(0)fprintf(stderr, " sum1 %.2f sum2 %.2f Nip %.2f N=%d ip=%.2f score %g\n", s1x, s2x, (double) N * ip, (int) N, ip, score)
         ;  mclvFree(&merge)
         ;  mclvFree(&veccx)
         ;  mclvFree(&vecdx)
      ;  }
         else
         {  double s2   =  sums->ivps[d].val
         ;  nom  =  nomleft * sqrt(Nssqs->ivps[d].val - s2*s2)
         ;  score=  nom ? ((N*ip - s1*s2) / nom) : 0.0
;if(0)fprintf(stderr, "score %g\n", score)
         ;  offending = nomleft ? d : c        /* prepare in case !nom */
      ;  }
      }

      *offendingp = offending
   ;  *nomp = nom
   ;  *scorep = score
   ;  return n_reduced
;  }


dim do_range_do
(  const mclx* tbl
,  mclx* res
,  mclx* mxna
,  double cutoff
,  dim start
,  dim end
,  mclv* Nssqs
,  mclv* sums
,  mcxbits bits
,  mcxbool lower_diagonal
,  dim thread_id
)
   {  dim c, p = 0
   ;  int n_mod =  MCX_MAX(1+ (MCX_MAX(1, n_thread_l) * 2 * (end - start -1))/40, 1)
   ;  mclv* scratch  =  mclvCopy(NULL, tbl->dom_cols)
   ;  dim n_reduced = 0
   ;  unsigned mink = bits & ARRAY_MINKOWSKI

   ;  for (c=start;c<end;c++)
      {  ofs s = 0
      ;  dim d_start = c, d_end = N_COLS(tbl)
      ;  dim d

      ;  if (!lower_diagonal)
            d_start = 0
         ,  d_end = c

      ;  for (d=d_start;d<d_end;d++)
         {  double score, absscore, nom
         ;  dim offending

;if(0)fprintf(stderr, "%d\t%d\n", (int) c, (int) d)
         ;  n_reduced += get_correlation(tbl, mxna, c, d, Nssqs, sums, &score, &nom, &offending, bits)

         ;  if ((bits & ARRAY_PEARSON) && !nom && tbl->dom_cols->ivps[offending].val < 1.5)
            {  char* label = tab_g ? mclTabGet(tab_g, offending, NULL) : NULL
            ;  if (label)
               mcxErr(me, "constant data for label <%s> - no pearson", label)
            ;  else
               mcxErr
               (  me
               ,  "constant data for %s %ld (mcl identifier %ld) - no pearson"
               ,  ((support_modalities & MODE_TRANSPOSE) ? "column" : "row")
               ,  (long) (offending+1)
               ,  (long) offending
               )
            ;  tbl->dom_cols->ivps[offending].val = 2
         ;  }

            absscore = score
         ;  if (absscore < 0)
            absscore *= -1

         ;  if (mink && !score)     /* we don't store zeroes .. */
            score = g_epsilon

         ;  if
            (  score
            && (  (!mink && absscore >= cutoff)
               || (mink  && (!cutoff || absscore <= cutoff))
               )
            )
               scratch->ivps[s].val = score
            ,  scratch->ivps[s].idx = tbl->cols[d].vid
            ,  s++
      ;  }

         {  dim n = scratch->n_ivps
         ;  scratch->n_ivps = s
         ;  mclvAdd(res->cols+c, scratch, res->cols+c)
         ;  res->cols[c].val = tbl->cols[c].n_ivps
         ;  scratch->n_ivps = n
      ;  }

         if (progress_g && (p+1) % n_mod == 0)
         fputc(thread_id < 10 ? '0' + thread_id : '.', stderr)
      ;  p++
   ;  }
      mclvFree(&scratch)
   ;  return n_reduced
;  }


dim do_range
(  const mclx* tbl
,  mclx* res
,  mclx* mxna
,  double cutoff
,  dim start
,  dim end
,  mclv* Nssqs
,  mclv* sums
,  mcxbits bits
,  dim thread_id
)
   {  dim n_reduced = 0, i

   ;  if (!end || end > N_COLS(tbl))
      end = N_COLS(tbl)
   
   ;  for (i=start;i<end;i++)
      {  if (res->dom_cols->ivps[i].val > 1.5)
         {  mcxErr(me, "overlap in range %u-%u", (unsigned) start, (unsigned) end)
         ;  break
      ;  }
         res->dom_cols->ivps[i].val = 2.0
   ;  }

      if (1 && (support_modalities & MODE_JOBINFO))
      {  fprintf(stdout, "%u\t%u\n", (unsigned) start, (unsigned) end)
      ;  return 0
   ;  }

      n_reduced
      =  do_range_do
         (  tbl
         ,  res
         ,  mxna
         ,  cutoff
         ,  start
         ,  end
         ,  Nssqs
         ,  sums
         ,  bits
         ,  TRUE
         ,  thread_id
         )
   ;  if (!sym_g)          /* result is not symmetric, so compute upper diagonal separately */
      n_reduced
      += do_range_do
         (  tbl
         ,  res
         ,  mxna
         ,  cutoff
         ,  start
         ,  end
         ,  Nssqs
         ,  sums
         ,  bits
         ,  FALSE
         ,  thread_id
         )
   ;  return n_reduced
;  }


typedef struct
{  const mclx* tbl
;  mclx*    res
;  mclx*    mxna
;  double   cutoff
;  dim      job_lo1
;  dim      job_lo2
;  dim      job_hi1
;  dim      job_hi2
;  dim      job_size
;  mclv*    Nssqs
;  mclv*    sums
;  dim      n_reduced
;  mcxbits  bits
;  dim      thread_id
;
}  array_data   ;


static void* array_thread
(  void* arg
)
   {  array_data* d = arg

   ;  if (d->job_lo2 > d->job_hi1)
      {  d->n_reduced
         =  do_range(d->tbl, d->res, d->mxna, d->cutoff, d->job_lo1, d->job_hi2, d->Nssqs, d->sums, d->bits, d->thread_id)
;if (0) fprintf(stderr, "thread %d %d-%d\n", (int) d->thread_id, (int) d->job_lo1, (int) d->job_hi2)
   ;  }
      else
      {  d->n_reduced
         += do_range(d->tbl, d->res, d->mxna, d->cutoff, d->job_lo1, d->job_lo2, d->Nssqs, d->sums, d->bits, d->thread_id)
      ;  d->n_reduced
         += do_range(d->tbl, d->res, d->mxna, d->cutoff, d->job_hi1, d->job_hi2, d->Nssqs, d->sums, d->bits, d->thread_id)
;if (0)
fprintf
(  stderr
,  "thread %d %d-%d and %d-%d\n"
,  (int) d->thread_id
,  (int) d->job_lo1
,  (int) d->job_lo2
,  (int) d->job_hi1
,  (int) d->job_hi2
)
   ;  }
      return NULL
;  }


mclx* normalise
(  mclx* tbl
,  const char* mode
)
   {  mcxbool secondary = strchr(mode, 'S') ? TRUE : FALSE
   ;  mcxbool bemean = strchr(mode, 'm') ? TRUE : FALSE
   ;  mclx* thetbl = secondary ? mclxTranspose(tbl) : tbl
   ;  double std, mean
   ;  dim c
   ;  for (c=0;c<N_COLS(thetbl);c++)
      {  mclvMean(thetbl->cols+c, N_ROWS(thetbl), &mean, &std)
      ;  mclvAffine(thetbl->cols+c, mean, bemean ? 1.0 : std)
   ;  }
      if (secondary)
      {  mclxFree(&tbl)
      ;  tbl = mclxTranspose(thetbl)
      ;  mclxFree(&thetbl)
   ;  }
      return tbl
;  }


void write_the_table
(  mclx* result
,  mcxIO* xf
,  int digits
,  mcxOnFail ON_FAIL
)
   {  mclxIOdumper dumper = { 0 }
   ;  mclxIOdumpSet(&dumper, MCLX_DUMP_VALUES | MCLX_DUMP_TABLE | MCLX_DUMP_TABLE_HEADER, "\t", "\t", "\t")
   ;  dumper.table_nlines  = 0
   ;  dumper.table_nfields = 0
   ;  dumper.prefixc = ""
   ;  dumper.siftype = NULL

   ;  if
      (  mclxIOdump
         (  result
         ,  xf
         ,  &dumper
         ,  tab_g
         ,  tab_g
         ,  digits
         ,  RETURN_ON_FAIL
       ) )
      mcxErr(me, "something suboptimal")
;  }


int main
(  int                  argc
,  const char*          argv[]
)
   {  int digits = MCLXIO_VALUE_GETENV
   ;  double cutoff = -1.0001
   ;  mclx* res
   ;  mcxIO* xfin = mcxIOnew("-", "r"), *xftab = NULL, *xftbl = NULL, *xfna = NULL
   ;  mclv* Nssqs, *sums
   ;  mcxbool read_table = FALSE
   ;  const char* mode_normalise = ""
   ;  mcxbool cutoff_specified = FALSE
   ;  mcxbool write_binary = FALSE, write_table = FALSE
   ;  const char* fnout = "-", *fnseql = NULL, *fnseqr = NULL
   ;  dim n_group_G = 1
   ;  dim i_group = 0
   ;  unsigned rskip = 0, cskip = 0, labelidx = 0
   ;  double N = 1.0
   ;  double acos_normalise = 0.0
   ;  mclgTF* tf_result  =   NULL, *tf_table = NULL
   ;  mcxTing* tf_result_spec = NULL, * tf_table_spec = NULL
   ;  mcxbits main_modalities = 0
   ;  mcxbits main_modalities_default = ARRAY_PEARSON
   ;  dim nrows_max = 0

   ;  mcxstatus parseStatus = STATUS_OK
   ;  mcxOption* opts, *opt
   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)
   
   ;  if
      (!(opts = mcxOptParse(options, (char**) argv, argc, 1, 0, &parseStatus)))
      exit(0)

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)
   ;  mclx_app_init(stderr)

   ;  for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = opt->anch

      ;  switch(anch->id)
         {  case MY_OPT_HELP
         :  mcxOptApropos(stdout, me, syntax, 0, MCX_OPT_DISPLAY_SKIP, options)

         ;  return 0
         ;

            case MY_OPT_NOTES
         :
puts("A and B are vectors");
puts("A /\\B denotes the indices on which both A and B are nonzero");
puts("A* denotes the subset of entries of A restricted to A/\\B");
puts("B* denotes the subset of entries of B restricted to A/\\B");
puts("");
puts("median:  min(median(A), median(B))     define the base score");
puts("average: min(avg(A*), avg(B*))         define the base score");
puts("cosine:  cosine(A*, B*)                reward coordinate pattern");
puts("euclid:  sqrt(sumsq(A*) / sumsq(A))    A* should be large compared to A");
puts("isect:   #A* / #A                      same as above, ignoring weights");
         ;  return 0
         ;

            case MY_OPT_T
         :  n_thread_l =  atoi(opt->val)
         ;  break
         ;

            case MY_OPT_JOBID
         :  i_group =  atoi(opt->val)
         ;  break
         ;

            case MY_OPT_NJOBS
         :  n_group_G =  atoi(opt->val)
         ;  break
         ;

            case MY_OPT_START
         :  start_g = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_END
         :  end_g = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_JI
         :  support_modalities |= MODE_JOBINFO
         ;  break
         ;

            case MY_OPT_TP
         :  support_modalities |= MODE_TRANSPOSE
         ;  break
         ;

            case MY_OPT_TABLE
         :  mcxIOnewName(xfin, opt->val)
         ;  read_table = TRUE
         ;  break
         ;

            case MY_OPT_L
         :  labelidx = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_ZEROASNA
         :  support_modalities |= MODE_ZEROASNA
         ;  break
         ;

            case MY_OPT_WRITE_NA
         :  xfna = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_LIMIT_ROWS
         :  nrows_max = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_WRITE_TABLE
         :  xftbl = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_TAB
         :  xftab = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_TABLETRANSFORM
         :  tf_table_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_TRANSFORM
         :  tf_result_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_DATA
         :  mcxIOnewName(xfin, opt->val)
         ;  break
         ;

            case MY_OPT_NOPROGRESS
         :  progress_g = FALSE
         ;  break
         ;

            case MY_OPT_SEQR
         :  fnseqr = opt->val
         ;  break
         ;

            case MY_OPT_SEQL
         :  fnseql = opt->val
         ;  break
         ;

            case MY_OPT_RSKIP
         :  rskip = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_CSKIP
         :  cskip = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_RANKTRANSFORM
         :  support_modalities |= MODE_RANKTRANSFORM
         ;  break
         ;

            case MY_OPT_NORMALISE
         :  mode_normalise = opt->val
         ;  break
         ;

            case MY_OPT_PEARSON
         :  main_modalities |= ARRAY_PEARSON
         ;  break
         ;

            case MY_OPT_SPEARMAN
         :  main_modalities |= ARRAY_SPEARMAN
         ;  support_modalities |= MODE_RANKTRANSFORM
         ;  break
         ;

            case MY_OPT_SUBSET_DIFF
         :  main_modalities |= ARRAY_SUBSET_DIFF
         ;  sym_g = FALSE
         ;  break
         ;

            case MY_OPT_SPARSE
         :  support_modalities |= MODE_SPARSE
         ;  break
         ;

            case MY_OPT_FINGERPRINT
         :  main_modalities |= ARRAY_FINGERPRINT
         ;  {  const char* c = opt->val
            ;  if (!strcmp(c, "tanimoto"))
               fingerprint_g = FP_TANIMOTO
            ;  else if (!strcmp(c, "cosine"))
               fingerprint_g = FP_COSINE
            ;  else if (!strcmp(c, "meet"))
               fingerprint_g = FP_MEET
            ;  else if (!strcmp(c, "hamming"))
               fingerprint_g = FP_HAMMING
            ;  else if (!strcmp(c, "cover"))
                  fingerprint_g = FP_COVER
               ,  sym_g = FALSE
            ;  else
               mcxDie(1, me, "unknown fingerprint mode %s", c)
         ;  }
            break
         ;

            case MY_OPT_CONTENTX
         :  main_modalities |= ARRAY_CONTENT
         ;  sym_g = FALSE
         ;  {  const char* c = opt->val
            ;  for(;c[0];c++)
               switch(c[0])
               {  case  'v' : contentx_g |= CONTENT_AVERAGE ;  break
               ;  case  'c' : contentx_g |= CONTENT_COSINE  ;  break
               ;  case  'e' : contentx_g |= CONTENT_EUCLID  ;  break
               ;  case  'i' : contentx_g |= CONTENT_INTERSECT; break
               ;  case  'm' : contentx_g |= CONTENT_MEDIAN  ;  break
               ;  default: mcxDie(1, me, "unsupported mode -content mode [%c]", (int) (unsigned char) c[0])
            ;  }
            }
            break
         ;

            case MY_OPT_NW
         :  support_modalities |= MODE_NOWRITE
         ;  break
         ;

            case MY_OPT_CONTENT
         :  main_modalities |= ARRAY_CONTENT
         ;  sym_g = FALSE
         ;  break
         ;

            case MY_OPT_SUBSET_MEET
         :  main_modalities |= ARRAY_SUBSET_MEET
         ;  sym_g = FALSE
         ;  break
         ;

            case MY_OPT_COSINE_SKEW
         :  main_modalities |= ARRAY_COSINESKEW
         ;  sym_g = FALSE
         ;  break
         ;

            case MY_OPT_DOT
         :  main_modalities |= ARRAY_DOT
         ;  break
         ;

            case MY_OPT_COSINE
         :  main_modalities |= ARRAY_COSINE
         ;  break
         ;

            case MY_OPT_MINKOWSKI
         :  case MY_OPT_EUCLID
         :  case MY_OPT_TAXI
         :  case MY_OPT_L00
         :  
            minkowski = 1.0
         ;  if (anch->id == MY_OPT_L00)
            minkowski = 0.0
         ;  else if (anch->id == MY_OPT_EUCLID)
            minkowski = 2.0
         ;  else if (anch->id == MY_OPT_MINKOWSKI)
            minkowski = atof(opt->val)
         ;  if (minkowski < 0)
            minkowski = 2.0
         ;  main_modalities |= ARRAY_MINKOWSKI
         ;  break
         ;

            case MY_OPT_ACUTE_ARC
         :  case MY_OPT_ACUTE_ARC_NORM
         :  main_modalities |= ARRAY_COSINE
         ;  main_modalities |= ARRAY_ARC
         ;  main_modalities |= ARRAY_ACUTE_ARC
         ;  if (anch->id == MY_OPT_ACUTE_ARC_NORM)
            acos_normalise = acos(0)
         ;  break
         ;

            case MY_OPT_ARC
         :  case MY_OPT_ARC_NORM
         :  main_modalities |= ARRAY_COSINE
         ;  main_modalities |= ARRAY_ARC
         ;  if (anch->id == MY_OPT_ARC_NORM)
            acos_normalise = acos(-1)
         ;  break
         ;

            case MY_OPT_SINE
         :  main_modalities |= ARRAY_COSINE
         ;  main_modalities |= ARRAY_SINE
         ;  break
         ;

            case MY_OPT_HSINE
         :  main_modalities |= ARRAY_COSINE
         ;  main_modalities |= ARRAY_HSINE
         ;  break
         ;

            case MY_OPT_HCOSINE
         :  main_modalities |= ARRAY_COSINE
         ;  main_modalities |= ARRAY_HCOSINE
         ;  break
         ;

            case MY_OPT_TEXTTABLE
         :  write_table = TRUE
         ;  break
         ;

            case MY_OPT_WRITEBINARY
         :  write_binary = TRUE
         ;  break
         ;

            case MY_OPT_AMOIXA
         :  mcxOptApropos
            (  stdout
            ,  me
            ,  NULL
            ,  15
            ,  MCX_OPT_DISPLAY_SKIP | MCX_OPT_DISPLAY_HIDDEN
            ,  options
            )
         ;  return 0
         ;

            case MY_OPT_VERSION
         :  app_report_version(me)
         ;  return 0
         ;

            case MY_OPT_CUTOFF
         :  cutoff = atof(opt->val)
         ;  cutoff_specified = TRUE
         ;  break
         ;

            case MY_OPT_O
         :  fnout = opt->val
         ;  break
         ;

            case MY_OPT_DIGITS
         :  digits = strtol(opt->val, NULL, 10)
         ;  break
      ;  }
      }

   ;  mcxOptFree(&opts)

   ;  if ((main_modalities & ARRAY_CONTENT) && !contentx_g)
      contentx_g = CONTENT_DEFAULT

   ;  if (n_thread_l && (start_g || end_g))
      mcxDie(1, me, "-start and -end do not mix with -t -J or -j")
                                    /* dangersign: start_g, threads, and groups */ 
   ;  if (n_group_G >= 1)
      {  if (!n_thread_l)
         n_thread_l = 1
      ;  n_thread_l = mclx_set_threads_or_die(me, n_thread_l, i_group, n_group_G)
   ;  }

      if (!main_modalities)
      main_modalities |= main_modalities_default

   ;  if (!cutoff_specified && !xftbl)
      mcxDie(1, me, "-co <cutoff> option is required")
   ;  if
      (  labelidx &&
         (  ((support_modalities & MODE_TRANSPOSE) && labelidx > rskip)
         || (!(support_modalities & MODE_TRANSPOSE) && labelidx > cskip)
         )
      )
      mcxDie(1, me, "-l value requires larger or equally large skipc/skipr argument")
   ;  else if ((support_modalities & MODE_TRANSPOSE) && xftab && !labelidx && rskip == 1)
      labelidx = 1
   ;  else if (!(support_modalities & MODE_TRANSPOSE) && xftab && !labelidx && cskip == 1)
      labelidx = 1
   ;  else if (xftab && !labelidx)
      mcxDie(1, me, "which column or row gives the label? Use -l")

   ;  if (tf_table_spec && !(tf_table = mclgTFparse(NULL, tf_table_spec)))
      mcxDie(1, me, "input -tf-table spec does not parse")
   ;  if (tf_result_spec && !(tf_result = mclgTFparse(NULL, tf_result_spec)))
      mcxDie(1, me, "input -tf spec does not parse")

   ;  mcxIOopen(xfin, EXIT_ON_FAIL)

   ;  if (xftab)
      mcxIOopen(xftab, EXIT_ON_FAIL)

   ;  if (fnseql && fnseqr)
      {  mcxIO* xf = mcxIOnew(fnseql, "r")
      ;  mxseql = mclxRead(xf, EXIT_ON_FAIL)
      ;  mcxIOclose(xf)
      ;  mcxIOrenew(xf, fnseqr, NULL)
      ;  mxseqr = mclxRead(xf, EXIT_ON_FAIL)
      ;  mcxIOfree(&xf)
   ;  }

      {  dim i
      ;  dim N_na = 0
      ;  mclx* mxna  =  NULL
      ;  mclx* tbl   =
                                                   /* fixme skeleton read makes subsequent code bit brittle */
               read_table                          /* fixme docme what is ARRAY_CONTENT about? */
               ?  (  (support_modalities & MODE_JOBINFO && !(main_modalities & ARRAY_CONTENT))
                  ?  mclxReadSkeleton(xfin, 0, FALSE)
                  :  mclxRead(xfin, EXIT_ON_FAIL)
                  )
               :  read_data(xfin, xftab, nrows_max, rskip, cskip, labelidx, main_modalities, &mxna)
      ;  mcxIOfree(&xfin)
      ;  if (!tbl)
         mcxDie(1, me, "no table")

#if 0
      ;  if (read_table && (support_modalities & MODE_ZEROASNA))
         {  mxna = mclxAllocZero(mclvClone(tbl->dom_cols), mclvClone(tbl->dom_rows))
         ;  for (i=0;i<N_COLS(tbl);i++)
            mcldMinus(tbl->dom_rows, tbl->cols+i, mxna->cols+i)
         ;  mcxTell(me, "have %d NAs from zero", (int) mclxNrofEntries(mxna))
      ;  }
#endif

      ;  if (tf_table)
         mclgTFexec(tbl, tf_table)

                                                            /* noteme added MODE_ZEROASNA below.
                                                             * interaction with mxna computation later
                                                            */
      ;  if
         (  !(main_modalities & (ARRAY_CONTENT | ARRAY_FINGERPRINT))
         && !(support_modalities & (MODE_SPARSE | MODE_ZEROASNA))
         )
         {  dim n_entries_in = mclxNrofEntries(tbl), n_entries_table = 0
         ;  for (i=0;i<N_COLS(tbl);i++)
            mclvCanonicalEmbed(tbl->cols+i, tbl->cols+i, N_ROWS(tbl), 0.0)
         ;  n_entries_table = mclxNrofEntries(tbl)
         ;  if (n_entries_table > n_entries_in)
            mcxTell
            (  me
            ,  "Expanded sparse table representation from %lu to %lu entries"
            ,  (long unsigned) n_entries_in
            ,  (long unsigned) n_entries_table
            )
      ;  }
         else
         mclxUnary(tbl, fltxCopy, NULL)    /* this removes zeroes from the matrix */

      ;  mcxTell(me, "%lu entries in table", (unsigned long) mclxNrofEntries(tbl))

      ;  if (read_table)
         mxna = mclxAllocClone(tbl)
      ;  else
         N_na = mclxNrofEntries(mxna)

      ;  if (N_na)
         mcxTell(me, "found a total of %lu NA/NaN entries", (ulong) N_na)

      ;  if (N_na && (support_modalities & MODE_ZEROASNA))
            mcxErr(me, "--zero-as-na ignored, real NAs found in data")
         ,  support_modalities ^= MODE_ZEROASNA

      ;  if (support_modalities & MODE_TRANSPOSE)
         {  mclx* tp = mclxTranspose(tbl)
         ;  mclx* tpna = mclxTranspose(mxna)
         ;  mclxFree(&tbl)
         ;  tbl = tp
         ;  mclxFree(&mxna)
         ;  mxna = tpna
      ;  }

         if (xfna && mxna)
         {  mclxWrite(mxna, xfna, digits, EXIT_ON_FAIL)
         ;  mcxIOfree(&xfna)
      ;  }

         if (strchr(mode_normalise, 'z'))
         tbl = normalise(tbl, mode_normalise)

                                             /* fixme funcify */
      ;  if (support_modalities & MODE_RANKTRANSFORM)
         {  dim d
         ;  rank_unit* ru
            =  mcxNAlloc
               (  N_ROWS(tbl) + 1            /* one extra for sentinel */
               ,  sizeof(rank_unit)
               ,  rank_unit_init
               ,  EXIT_ON_FAIL
               )
         ;  for (d=0;d<N_COLS(tbl);d++)
            {  mclv* v = tbl->cols+d
            ;  dim stretch = 1
            ;  dim i
            ;  for (i=0;i<v->n_ivps;i++)
               {  ru[i].index = v->ivps[i].idx
               ;  ru[i].value = v->ivps[i].val
               ;  ru[i].ord = -14
            ;  }
               qsort(ru, v->n_ivps, sizeof ru[0], rank_unit_cmp_value)
            ;  ru[v->n_ivps].value = ru[v->n_ivps-1].value * 1.1    /* sentinel out-of-bounds value */
            ;  for (i=0;i<v->n_ivps;i++)
               {  if (ru[i].value != ru[i+1].value)    /* input current stretch */
                  {  dim j
                  ;  for (j=0;j<stretch;j++)
                     ru[i-j].ord = 1 + (i+1 + i-stretch) / 2.0
                  ;  stretch = 1
               ;  }
                  else
                  stretch++
            ;  }
               qsort(ru, v->n_ivps, sizeof ru[0], rank_unit_cmp_index)
            ;  for (i=0;i<v->n_ivps;i++)
               v->ivps[i].val = ru[i].ord
;if(0)fprintf(stderr, "vid %d id %d value from %.4g to %.1f\n", (int) v->vid, (int) v->ivps[i].idx, (double) ru[i].value, (double) ru[i].ord)
         ;  }
         }

         if (strchr(mode_normalise, 'm'))
         tbl = normalise(tbl, mode_normalise)

      ;  if (support_modalities & MODE_ZEROASNA)
         {  dim i
         ;  if (main_modalities & (ARRAY_COSINESKEW | ARRAY_SUBSET_MEET | ARRAY_SUBSET_DIFF | ARRAY_CONTENT | ARRAY_FINGERPRINT))
            mcxDie(1, me, "--zero-as-na only supported with --spearman or --pearson or --cosine")
/* fixme: zero-as-na supported when exactly? */
         ;  if (!(main_modalities & (ARRAY_SPEARMAN | ARRAY_PEARSON | ARRAY_COSINE)))
            mcxDie(1, me, "--zero-as-na only supported with --spearman or --pearson or --cosine")
         ;  for (i=0;i<N_COLS(tbl);i++)
            mcldMinus(tbl->dom_rows, tbl->cols+i, mxna->cols+i)
         ;  mclxUnary(tbl, fltxCopy, NULL)    /* this removes zeroes from the matrix */
         ;  mcxTell(me, "have %d NAs from zero", (int) mclxNrofEntries(mxna))
      ;  }
         else
         {  if (main_modalities & (ARRAY_SUBSET_DIFF | ARRAY_SUBSET_MEET | ARRAY_CONTENT | ARRAY_FINGERPRINT))
            mclxUnary(tbl, fltxCopy, NULL)    /* this removes zeroes from the matrix */
         ;  else
            {  dim i
            ;  for (i=0;i<N_COLS(tbl);i++)
               {  if (tbl->cols[i].n_ivps != N_ROWS(tbl))
                  mclvCanonicalEmbed(tbl->cols+i, tbl->cols+i, N_ROWS(tbl), 0.0)
            ;  }
            }
         }

         if (xftbl)
         {  if (write_binary)
            mclxbWrite(tbl, xftbl, RETURN_ON_FAIL)
         ;  else
            mclxWrite(tbl, xftbl, digits, RETURN_ON_FAIL)
         ;  mcxIOfree(&xftbl)
         ;  exit(0)
      ;  }

         Nssqs    =  mclvCopy(NULL, tbl->dom_cols)
      ;  sums     =  mclvCopy(NULL, tbl->dom_cols)

      ;  N  = MCX_MAX(N_ROWS(tbl), 1)      /* fixme; bit odd */

      ;  {  dim c
         ;  for (c=0;c<N_COLS(tbl);c++)
            {  double sumsq = mclvPowSum(tbl->cols+c, 2.0)
            ;  double sum = mclvSum(tbl->cols+c)
            ;  Nssqs->ivps[c].val = N * sumsq
            ;  sums->ivps[c].val = sum
         ;  }
         }

         res   =
         mclxAllocZero
         (  mclvCopy(NULL, tbl->dom_cols)
         ,  mclvCopy(NULL, tbl->dom_cols)
         )

      ;  {  dim n_reduced =0
;if(0)fprintf(stderr, "%d %d\n", (int) n_thread_l, (int) n_group_G)
         ;  if (n_thread_l * n_group_G <= 1)
            n_reduced = do_range(tbl, res, mxna, cutoff, start_g, end_g, Nssqs, sums, main_modalities, 0)

         ;  else
            {  struct jobinfo ji
            ;  dim t_this_group = 0, t_max = 0, t = 0

                           /* Computation of the group of threads for a job
                            * is somewhat complicated by our mini-job balancing as
                            * described above.
                           */
            ;  pthread_t *threads_array
               =  mcxAlloc(n_thread_l * sizeof threads_array[0], EXIT_ON_FAIL)
            ;  array_data* data = mcxAlloc(n_thread_l * sizeof data[0], EXIT_ON_FAIL)

            ;  pthread_attr_t  t_attr
            ;  pthread_attr_init(&t_attr)

                           /* fixme docme conditions under which last argument to ji_init
                            * is effective (i.e. by_set_size works effectively)
                           */
            ;  ji_init
               (  &ji
               ,  tbl
               ,  n_thread_l
               ,  n_group_G
               ,  i_group
               ,  (main_modalities & (ARRAY_FINGERPRINT | ARRAY_CONTENT | ARRAY_SUBSET_MEET | ARRAY_SUBSET_DIFF))
               )

            ;  while (ji.dvd_joblo2 < ji.dvd_jobhi1)
               {  array_data* d =  data+t_this_group

               ;  if (!ji_step(&ji, t++, t_this_group))
                  continue

               ;  d->tbl      =  tbl
               ;  d->res      =  res
               ;  d->mxna     =  mxna
               ;  d->cutoff   =  cutoff
               ;  d->job_lo1  =  ji.dvd_joblo1
               ;  d->job_lo2  =  ji.dvd_joblo2
               ;  d->job_hi1  =  ji.dvd_jobhi1
               ;  d->job_hi2  =  ji.dvd_jobhi2
               ;  d->job_size =  ji.dvd_jobsize
               ;  d->Nssqs    =  Nssqs
               ;  d->sums     =  sums
               ;  d->bits     =  main_modalities
               ;  d->n_reduced=  0
               ;  d->thread_id=  t_this_group

               ;  if (pthread_create(threads_array+t_this_group, &t_attr, array_thread, d))
                  mcxDie(1, me, "error creating thread %d", (int) t_this_group)
               ;  t_this_group++
               ;  if (t_this_group > n_thread_l)
                  mcxDie(1, me, "thread worker distribution off colour")
            ;  }
               t_max = t_this_group                      /* fixme or document */
            ;  for (t=0; t < t_max; t++)
               pthread_join(threads_array[t], NULL)
            ;  for (t=0; t < t_max; t++)
               n_reduced += data[t].n_reduced
            ;  mcxFree(threads_array)
            ;  mcxFree(data)
         ;  }

            if (support_modalities & MODE_JOBINFO)
            return 0

         ;  if (progress_g)
            fputc('\n', stderr)
         ;  if (N_na)
            {  dim i,  n_row_with_na = 0
            ;  for (i=0;i<N_COLS(mxna);i++)
               if (mxna->cols[i].n_ivps)
               n_row_with_na++
            ;  mcxTell
               (  me
               ,  "fraction of computations involving NA: %.2f"
               ,  (double) ((2.0 * n_reduced - n_row_with_na) / (N_COLS(tbl) * 1.0 * N_COLS(tbl)))
               )
            ,  mcxTell
               (  me
               ,  "number of rows with NA: %d"
               ,  (int) n_row_with_na
               )
         ;  }

            else if (support_modalities & MODE_ZEROASNA)
            {  dim i,  n_row_with_na = 0
            ;  for (i=0;i<N_COLS(tbl);i++)
               if (tbl->cols[i].n_ivps != N_ROWS(tbl))
               {  n_row_with_na++
            ;  }
               mcxTell
               (  me
               ,  "fraction of computations involving zero-as-NA: %.2f"
               ,  (double) ((2.0 * n_reduced - n_row_with_na) / (N_COLS(tbl) * N_COLS(tbl)))
               )
            ,  mcxTell
               (  me
               ,  "number of rows with zero-as-NA: %d (reduced %d)"
               ,  (int) n_row_with_na
               ,  (int) n_reduced
               )
         ;  }
         }

         if (sym_g)
         mclxAddTranspose(res, 0.5)
      ;  mclxFree(&tbl)
      ;  mclxFree(&mxna)
   ;  }

      if (tf_result)
      mclgTFexec(res, tf_result)

   ;  if (acos_normalise)
      mclxUnary(res, fltxScale, &acos_normalise)

   ;  if (support_modalities & MODE_NOWRITE)
      return 0

   ;  mclvFree(&Nssqs)
   ;  mclvFree(&sums)
   ;

      {  mcxIO* xfout = mcxIOnew(fnout, "w")
      ;  if (write_table)
         write_the_table(res, xfout, digits, RETURN_ON_FAIL)
      ;  else if (write_binary)
         mclxbWrite(res, xfout, RETURN_ON_FAIL)
      ;  else
         mclxWrite(res, xfout, digits, RETURN_ON_FAIL)
      ;  mcxIOfree(&xfout)
   ;  }

      mclxFree(&res)
   ;  mclxFree(&mxseqr)
   ;  mclxFree(&mxseql)
   ;  return 0
;  }



/*
   M use median
   V use average
   S use sum(fabs)
   E use sqrt(ssq)
   N use #entries
   A  vector A
   a  vector A*
   B  vector B
   b  vector B*
   c  cosine(a,b)
   +
   /
   *
   r
*/


