/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef mcx_ops_h_h__
#define mcx_ops_h_h__

#include "util/ting.h"


extern unsigned n_threads_g;


#define     TOKEN_WHILE              "while"
#define     TOKEN_REPEAT             "repeat"
#define     TOKEN_DO                 "do"
#define     TOKEN_IFELSE             "ifelse"

#define     TOKEN_DIGITS             "dgt"

#define     TOKEN_DUP                "dup"
#define     TOKEN_COPY               "copy"
#define     TOKEN_MDUP               "mdup"
#define     TOKEN_EXCH               "exch"
#define     TOKEN_POP                "pop"
#define     TOKEN_ROLL               "roll"
#define     TOKEN_CLEAR              "clear"

#define     TOKEN_BLOCK              "block"

#define     TOKEN_MUL                "mul"
#define     TOKEN_ADD                "add"
#define     TOKEN_ADDTP              "addtp"
#define     TOKEN_ADDTO              "addto"
#define     TOKEN_POW                "pow"
#define     TOKEN_MIN                "min"
#define     TOKEN_MAX                "max"
#define     TOKEN_DIV                "div"
#define     TOKEN_MOD                "mod"

#define     TOKEN_LOADFILE           "lf"
#define     TOKEN_DEF                "def"
#define     TOKEN_FREE               "free"
#define     TOKEN_UNLINK             "unlink"

#define     TOKEN_EQ                 "eq"
#define     TOKEN_LT                 "lt"
#define     TOKEN_LQ                 "lq"
#define     TOKEN_GT                 "gt"
#define     TOKEN_GQ                 "gq"

#define     TOKEN_LIST               "list"
#define     TOKEN_TELL               "tell"
#define     TOKEN_OPLIST             "ops"
#define     TOKEN_HELP               "help"
#define     TOKEN_INFO               "info"
#define     TOKEN_TUT                "tut"
#define     TOKEN_SEARCH             "grep"
#define     TOKEN_VARS               "vars"
#define     TOKEN_QUIT               "quit"
#define     TOKEN_DONG               "vb"

#define     TOKEN_IDENTITY           "id"
#define     TOKEN_ALLONE             "jj"
#define     TOKEN_LOADMATRIX         "lm"
#define     TOKEN_HADAMARD           "hdm"
#define     TOKEN_COLSUMS            "colsums"
#define     TOKEN_NEW                "new"
#define     TOKEN_SET                "set"
#define     TOKEN_IMAC               "imac"

#define     TOKEN_MAKECHARACTERISTIC "ch"
#define     TOKEN_MAKESTOCHASTIC     "st"
#define     TOKEN_SELECT             "sel"
#define     TOKEN_SELECTC            "selc"
#define     TOKEN_TRANSFORM          "tf"
#define     TOKEN_INFLATE            "infl"
#define     TOKEN_EXPAND             "xpn"
#define     TOKEN_HADAMARDPOWER      "hdp"

#define     TOKEN_THREADS            "threads"

#define     TOKEN_ROWDIMENSION       "rdim"
#define     TOKEN_COLDIMENSION       "cdim"
#define     TOKEN_DIMENSION          "dim"

#define     TOKEN_SIZE               "size"

#define     TOKEN_TRANSPOSE          "tp"

#define     TOKEN_VIEWMATRIX         "vm"
#define     TOKEN_WRITEMATRIX        "wm"

void opInitialize(void)  ;
void opExit(void);

typedef  int (*opFunc)(void) ;

opFunc opGetOpByToken
(  mcxTing* token
)  ;

#endif


