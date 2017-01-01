/*   (C) Copyright 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include "util/ting.h"
#include "impala/matrix.h"

#define SHCL_ERROR_PARTITION  1
#define SHCL_ERROR_DOMAIN     2
#define SHCL_ERROR_OTHER      4

void report_partition
(  const char* me
,  mclMatrix* cl
,  mcxTing* fname
,  dim o
,  dim m
,  dim e
)  ;

void report_exit
(  const char* me
,  mcxbits error
)  ;

void report_domain
(  const char* me
,  int nlft
,  int nrgt
,  int meet
)  ;


void report_fixit
(  const char* me
,  int n_fix
)  ;


