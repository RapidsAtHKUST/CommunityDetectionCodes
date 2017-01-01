/*   (C) Copyright 2008 Stijn van Dongen
 *
 * This file is part of mcl.  You can redistribute and/or modify mcl
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with mcl, in the file COPYING.
*/

#ifndef clm_h
#define clm_h

#include "util/opt.h"
#include "util/types.h"

enum
{  CLM_DISP_HELP = 0
,  CLM_DISP_APROPOS
,  CLM_DISP_VERSION
,  CLM_DISP_TEST
,  CLM_DISP_DEBUG
,  CLM_DISP_DEBUG2
,  CLM_DISP_PROGRESS
,  CLM_DISP_PROGRESS2
,  CLM_DISP_SET
,  CLM_DISP_NOP
,  CLM_DISP_AMOIXA
,  CLM_DISP_UNUSED = CLM_DISP_AMOIXA + 2
}  ;

extern mcxbits clm_debug_g;
extern unsigned clm_progress_g;
extern mcxbool clm_test_g;

#endif


