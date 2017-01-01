/*   (C) Copyright 2008 Stijn van Dongen
 *
 * This file is part of mcl.  You can redistribute and/or modify mcl
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with mcl, in the file COPYING.
*/

#ifndef mcx_h
#define mcx_h

#include "util/opt.h"
#include "util/io.h"
#include "util/types.h"

#include "impala/matrix.h"
#include "impala/tab.h"

#include "mcl/transform.h"

enum
{  MCX_DISP_HELP = 0
,  MCX_DISP_APROPOS
,  MCX_DISP_VERSION
,  MCX_DISP_WB
,  MCX_DISP_PROGRESS
,  MCX_DISP_PROGRESS2
,  MCX_DISP_SET
,  MCX_DISP_NOP
,  MCX_DISP_TEST
,  MCX_DISP_DEBUG
,  MCX_DISP_DEBUG2
,  MCX_DISP_AMOIXA
,  MCX_DISP_UNUSED = MCX_DISP_AMOIXA + 2
}  ;


extern mcxbits mcx_debug_g;
extern unsigned mcx_progress_g;
extern mcxbool mcx_test_g;
extern mcxbool mcx_wb_g;

                              /* in label case always returns a graph */
mclx* mcx_get_graph
(  const char* caller
,  mcxIO*   xfmx
,  mcxIO*   xfabc
,  mcxIO*   xftab
,  mclTab** tabpp
,  mclgTF*  transform
,  mcxbits  readx_bits        /* MCLX_REQUIRE_CANONICAL MCLX_REQUIRE_GRAPH */
)  ;


mcxstatus mcx_dump_node
(  FILE* fp
,  const mclTab* tab
,  long idx
)  ;


#endif


