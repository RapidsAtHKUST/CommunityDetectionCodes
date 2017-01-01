/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef mcl_interpret_h
#define mcl_interpret_h

#include "impala/matrix.h"

typedef struct
{  double   w_selfval      /* default ~ 0.001   */
;  double   w_maxval       /* default ~ 0.999   */
;  double   delta          /* default 0.01      */
;  
}  mclInterpretParam;

mclInterpretParam* mclInterpretParamNew
(  void
)  ;

void mclInterpretParamFree
(  mclInterpretParam **ipp
)  ;

mclMatrix* mclInterpret
(  mclMatrix*     mx
)  ;

mclMatrix* mclDag
(  const mclMatrix* A
,  const mclInterpretParam* ipp
)  ;

int mclDagTest
(  const mclMatrix* dag
)  ;

void  clusterMeasure
(  const mclMatrix*     clus
,  FILE*                fp
)  ;

mclVector*  mcxAttractivityScale
(  const mclMatrix*     M
)  ;


#if 0
void mcxDiagnosticsAttractor
(  const char*          ffn_attr
,  const mclMatrix*     clustering
,  const mcxDumpParam*  dumpParam
)  ;


void mcxDiagnosticsPeriphery
(  const char*          ffn_peri
,  const mclMatrix*     clustering
,  const mcxDumpParam*  dumpParam
)  ;
#endif


#endif

