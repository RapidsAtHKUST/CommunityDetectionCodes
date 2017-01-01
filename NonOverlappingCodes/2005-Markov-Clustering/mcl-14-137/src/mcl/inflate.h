/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef mcl_inflate_h
#define mcl_inflate_h

#include "impala/matrix.h"
#include "proc.h"

void mclxInflateBoss
(  mclMatrix*        mx
,  double            power
,  mclProcParam*     mpp
)  ;

void  mclvInflateLine
(  void *arg
)  ;

#endif

