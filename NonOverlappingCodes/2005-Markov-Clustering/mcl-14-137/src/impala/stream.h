/*   (C) Copyright 2005, 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef impala_stream_h
#define impala_stream_h

#include "ivp.h"
#include "vector.h"
#include "matrix.h"
#include "tab.h"

#include "util/io.h"
#include "util/types.h"
#include "util/ting.h"

mcxstatus mclxIOstreamOut
(  const mclx* mx
,  mcxIO* xf
,  mcxOnFail ON_FAIL
)  ;


#define MCLXIO_STREAM_PACKED        1 <<  0
#define MCLXIO_STREAM_ABC           1 <<  1
#define MCLXIO_STREAM_123           1 <<  2

#define MCLXIO_STREAM_ETC           1 <<  3     /* arbitrary nr of labels per line */
#define MCLXIO_STREAM_ETC_AI        1 <<  4     /* autoincrement: no column labels */

#define MCLXIO_STREAM_235           1 <<  5     /* arbitrary nr of numbers per line*/
#define MCLXIO_STREAM_235_AI        1 <<  6     /* autoincrement: no column labels */

#define MCLXIO_STREAM_READ  (MCLXIO_STREAM_PACKED | MCLXIO_STREAM_ABC | MCLXIO_STREAM_123 | MCLXIO_STREAM_ETC)
#define MCLXIO_STREAM_NUMERIC (MCLXIO_STREAM_123 | MCLXIO_STREAM_235 | MCLXIO_STREAM_235_AI)

#define MCLXIO_STREAM_WARN          1 <<  7     /* a/1 warn on parse miss */
#define MCLXIO_STREAM_STRICT        1 <<  8     /* a/1 fail on parse miss */

#define MCLXIO_STREAM_MIRROR        1 <<  9     /* seeing x-y-f, add y-x-f */
#define MCLXIO_STREAM_SYMMETRIC     1 << 10     /* domains are the same */
#define MCLXIO_STREAM_DEBUG         1 << 11     /* debug */

#define MCLXIO_STREAM_CTAB_EXTEND   1 << 12     /* on miss extend tab */
#define MCLXIO_STREAM_CTAB_STRICT   1 << 13     /* on miss fail */
#define MCLXIO_STREAM_CTAB_RESTRICT 1 << 14     /* on miss ignore */
#define MCLXIO_STREAM_RTAB_EXTEND   1 << 15     /* on miss extend tab */
#define MCLXIO_STREAM_RTAB_STRICT   1 << 16     /* on miss fail */
#define MCLXIO_STREAM_RTAB_RESTRICT 1 << 17     /* on miss ignore */

#define MCLXIO_STREAM_LOGTRANSFORM     1 << 18
#define MCLXIO_STREAM_NEGLOGTRANSFORM  1 << 19
#define MCLXIO_STREAM_LOG10         1 << 20
#define MCLXIO_STREAM_SIF           1 << 21
#define MCLXIO_STREAM_EXPECT_VALUE  1 << 22

#define MCLXIO_STREAM_GTAB_EXTEND (MCLXIO_STREAM_RTAB_EXTEND | MCLXIO_STREAM_CTAB_EXTEND)
#define MCLXIO_STREAM_GTAB_RESTRICT (MCLXIO_STREAM_RTAB_RESTRICT | MCLXIO_STREAM_CTAB_RESTRICT)
#define MCLXIO_STREAM_GTAB_STRICT (MCLXIO_STREAM_RTAB_STRICT | MCLXIO_STREAM_CTAB_STRICT)



/* In abc mode, it tries to separate on tab if it spots a tab;
 * otherwise it separates on whitespace.
 *
 * This interface is underdocumented. fixme.
*/

typedef struct
{  mclTab*        tab_sym_in
;  mclTab*        tab_sym_out
;  mclTab*        tab_col_in
;  mclTab*        tab_col_out
;  mclTab*        tab_row_in
;  mclTab*        tab_row_out
;  dim            cmax_123
;  dim            rmax_123
;  dim            cmax_235
;
}  mclxIOstreamer ;


/* In symmetric mode, tab_sym_out will be a newly created tab.
 * Otherwise, tab_{col,row}_out will be two newly created tabs.
 *    however, no new tab if an input tab was provided an the
 *    number of entries did not change.
 *    This is a Bad Bad interface. TODO.
*/

mclx* mclxIOstreamIn
(  mcxIO* xf
,  mcxbits  bits
,  mclpAR*  transform
,  void (*ivpmerge)(void* ivp1, const void* ivp2)
,  mclxIOstreamer* streamer
,  mcxOnFail ON_FAIL
)  ;


#endif

