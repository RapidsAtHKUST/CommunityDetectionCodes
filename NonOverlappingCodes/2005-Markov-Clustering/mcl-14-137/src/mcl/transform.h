/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#ifndef mcl_transform_h
#define mcl_transform_h

#include "util/list.h"
#include "util/ting.h"

#include "impala/matrix.h"

typedef struct mclg_transform mclgTF;

/* supply either encoding_link OR encoding_ting.
   ting e.g.: log(3), ceil(5), gq(2)
   link e.g.: mcxTokArgs of the above.
*/

mclgTF* mclgTFparse
(  mcxLink*    encoding_link
,  mcxTing*    encoding_ting
)  ;

dim mclgTFexec
(  mclx*    mx
,  mclgTF*  tf
)  ;

dim mclgTFexecx
(  mclx*    mx
,  mclgTF*  tf
,  mcxbool  allow_graph_ops
)  ;

void mclgTFfree
(  mclgTF** tfpp
)  ;

mclpAR* mclgTFgetEdgePar
(  mclgTF* tf
)  ;


#endif


