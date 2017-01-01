#!/usr/local/bin/bash
set -e


prefix=$HOME/local

if [[ ! -e $prefix/lib/libimpala.a ]]; then
   echo "Set prefix so that prefix/lib/libimpala.a exists"
   false
fi

if [[ ! -e $prefix/include/mcl/impala/ivp.h ]]; then
   echo "Set prefix so that prefix/include/mcl/ exists"
   echo "and contains impala/ivp.h impala/ivptypes.h util/inttypes.h util/types.h"
   false
fi


cat <<EOC >mcx-test-packed.c

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <string.h>


#include "impala/ivp.h"

int main(int argc, char **argv)
   {  mclp small[12][4] = {  { { 1, 1.0 }, { 5, 1.0 }, { 6, 1.0 } , { 9, 1.0 } }
                           , { { 0, 1.0 }, { 2, 1.0 }, { 4, 1.0 } ,                 {-1, 1.0 } }
                           , { { 1, 1.0 }, { 3, 1.0 }, { 4, 1.0 } ,                 {-1, 1.0 } }
                           , { { 2, 1.0 }, { 7, 1.0 }, { 8, 1.0 } , {10, 1.0 } }
                           , { { 1, 1.0 }, { 2, 1.0 }, { 6, 1.0 } , { 7, 1.0 } }
                           , { { 0, 1.0 }, { 9, 1.0 },                              {-1, 1.0 } , {-1, 1.0 } }
                           , { { 0, 1.0 }, { 4, 1.0 }, { 9, 1.0 } ,                 {-1, 1.0 } }
                           , { { 3, 1.0 }, { 4, 1.0 }, { 8, 1.0 } , {10, 1.0 } }
                           , { { 3, 1.0 }, { 7, 1.0 }, {10, 1.0 } , {11, 1.0 } }
                           , { { 0, 1.0 }, { 5, 1.0 }, { 6, 1.0 } ,                 {-1, 1.0 } }
                           , { { 3, 1.0 }, { 7, 1.0 }, { 8, 1.0 } , {11, 1.0 } }
                           , { { 8, 1.0 }, {10, 1.0 },                              {-1, 1.0 } , {-1, 1.0 } }
                           }
   ;  long src_and_len[2]
   ;  long sizes[12] = { 4, 3, 3, 4, 4, 2, 3, 4, 4, 3, 4, 2 }
   ;  long i

   ;  mclp ivp = { 1, 1.0 }
   ;  for (i=0; i<12; i++)
      {  src_and_len[0] = i
      ;  src_and_len[1] = sizes[i]
                        /*  Write source node and number of neighbours specified; two longs */
      ;  if (2 != fwrite(src_and_len, sizeof src_and_len[0], 2, stdout))
         exit(13)
                        /*  A list of ivps (index value pairs), each containing a neighbour index and a weight */
      ;  if (sizes[i] != fwrite(small[i], sizeof ivp, sizes[i], stdout))
         exit(13)
   ;  }
      return 0
;  }
EOC

gcc -g -I$HOME/local/include/mcl -c mcx-test-packed.c
gcc -g -L$HOME/local/lib -limpala -o mcx-test-packed mcx-test-packed.o

./mcx-test-packed | mcxload -packed - -pack-cnum 12 -pack-rnum 12 -ri max


