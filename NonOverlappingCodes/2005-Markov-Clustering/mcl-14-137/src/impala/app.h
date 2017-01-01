/*   (C) Copyright 2005, 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef impala_app_h__
#define impala_app_h__

#include "util/inttypes.h"

void app_report_version
(  const char* me
)  ;

void mclxSetBinaryIO
(  void
)  ;

void mclxSetInterchangeIO
(  void
)  ;

void mclx_app_init
(  FILE* fp
)  ;

dim mclx_set_threads_or_die
(  const char* caller
,  dim n_thread_l
,  dim i_group_G
,  dim n_group_G
)  ;

#endif
