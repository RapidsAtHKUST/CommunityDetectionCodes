/*   (C) Copyright 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include "mcl/proc.h"

#include "util/types.h"
#include "util/hash.h"
#include "util/opt.h"


#ifndef mcl_init_h
#define mcl_init_h

extern mcxOptAnchor mclProcOptions[];

mcxstatus mclProcessInit
(  const mcxOption*   opts
,  mcxHash*           theanchs
,  mclProcParam*      mp
)  ;

void mclProcOptionsInit
(  void
)  ;

void  mclSetProgress
(  int   n_nodes
,  mclProcParam* mpp
)  ;

void mclShowSettings (FILE* fp, mclProcParam* mpp, mcxbool user);
void mclShowSchemes(mcxbool print_skid);

void doBool(const char *string, mclProcParam* mp);
int  doInfoFlag(const char *string, mclProcParam* mp);
int  flagWithArg(const int a, const char *string);
void toggle(int* i, const char* s);
void usage();


#endif

