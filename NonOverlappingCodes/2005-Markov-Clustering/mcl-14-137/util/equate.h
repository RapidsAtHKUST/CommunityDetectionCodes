/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef equate_h
#define equate_h

int intCmp
(  const void*          i1
,  const void*          i2
)  ;

int intRevCmp
(  const void*          i1
,  const void*          i2
)  ;

int intnCmp
(  const int*           i1
,  const int*           i2
,  int   n
)  ;

int fltCmp
(  const void*          f1
,  const void*          f2
)  ;

int fltRevCmp
(  const void*          f1
,  const void*          f2
)  ;

int dblCmp
(  const void*          f1      
,  const void*          f2
)  ;

int dblRevCmp
(  const void*          f1      
,  const void*          f2
)  ;

int intLt
(  const void*          i1
,  const void*          i2
)  ;

int intLq
(  const void*          i1
,  const void*          i2
)  ;

int intGt
(  const void*          i1
,  const void*          i2
)  ;

int intGq
(  const void*          i1
,  const void*          i2
)  ;

int fltLt
(  const void*          f1
,  const void*          f2
)  ;

int fltLq
(  const void*          f1
,  const void*          f2
)  ;

int fltGt
(  const void*          f1
,  const void*          f2
)  ;

int fltGq
(  const void*          f1
,  const void*          f2
)  ;

#endif

