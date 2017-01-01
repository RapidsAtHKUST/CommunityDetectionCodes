#!/usr/local/bin/bash

export MCLXIOVERBOSITY=2     # 2=forced-silent 8=forced-verbose
export TINGEA_LOG_TAG=x

set -e
# set -x

load='mx/mcxload'

NUM=5
have_arg=$#
REPEAT=1

if let $(($#)); then
   REPEAT=$1
fi

function out {
   s=$?
   if let $(($s)); then
      echo "error occurred! in cmode=$cmode rmode=$rmode"
   elif ! let $(($have_arg)); then
      rm -f xxx.* yyy.*
   fi
}

trap out SIGTERM # EXIT

for ((q=0;$q<$REPEAT;q=$(($q+1)))); do
echo "--- $q ---"
   for rmode in restrict extend; do
      for cmode in restrict extend; do
         for ((i=1;$i<=$NUM;i=$(($i+1)))); do

         N=$((20*$i))         # generate N lines
         k=$((2*$i))          # with k
         s=$((1*$i))          # with s sdev
         echo "generate $N $k $s"

         stem="xxx.$cmode.$rmode"

         # generate etc file.
         ./abc.pl $N $k $s > $stem.raw

         # load it, with various tab modes.
         $load -etc $stem.raw \
            -$rmode-tabr az.tab -write-tabr $stem.tabr \
            -$cmode-tabc az.tab -write-tabc $stem.tabc \
            -o $stem.mci > /dev/null

         # dump it with standard mcxdump.
         mcxdump -imx $stem.mci -tabr $stem.tabr -tabc $stem.tabc --no-values --lazy-tab | sort > $stem.dump.mci

         # dump the original input under the same extend/restrict conditions.
         ./abc-pairs.pl --rmode=$rmode --cmode=$cmode $stem.raw | sort -u > $stem.dump.raw

         # reload the loaded/dumped input with strict tabs; output is not further tested.
         $load --stream-split -abc $stem.dump.mci -strict-tabr $stem.tabr -strict-tabc $stem.tabc -o /dev/null > /dev/null

         # diff the abc-pairs and mcxload outputs.
         if diff -q $stem.dump.raw $stem.dump.mci; then
            echo "/ $rmode $cmode $i ok"
         else
            echo "/ $rmode $cmode $i error"
            false
         fi

      ##
      ## now, similarly for etc-ai mode
      ##

         rawfile=yyy.$rmode.raw
         cp  $stem.raw  $rawfile
         stem="yyy.$rmode"

         $load -etc-ai $rawfile \
            -$rmode-tabr az.tab -write-tabr $stem.tabr \
            -o $stem.mci > /dev/null
         mcxdump -imx $stem.mci -tabr $stem.tabr --no-values --lazy-tab | sort > $stem.dump.mci
         ./abc-pairs.pl --etcai --rmode=$rmode $rawfile | sort -u > $stem.dump.raw
         if diff -q $stem.dump.raw $stem.dump.mci; then
            echo "/ etcai $rmode $i ok"
         else
            echo "/ etcai $rmode $i error"
            false
         fi

         done

      # Originally we had something here to check symmetric load with etc.

      done
   done
done



# for rmode in restrict extend; do
#    for cmode in restrict extend; do
# 
#    for ((i=1;$i<=$NUM;i=$(($i+1)))); do
# 
#    N=$((100*$i))
# 
#    stem="yyy"
#    ./abc.pl $N 1 0 > $stem.raw
# 
#    $load -abc $stem.raw \
#       -$rmode-tabr az.tab -write-tabr $stem.rtab \
#       -$cmode-tabc az.tab -write-tabc $stem.ctab \
#       -o $stem.mci > /dev/null
#    mcxdump -imx $stem.mci -tabr $stem.rtab -tabc $stem.ctab --no-values | sort > $stem.dump.mci
#    $load -abc $stem.dump.mci -strict-tabc $stem.ctab -strict-tabr $stem.rtab -o /dev/null
#    ./abc-pairs.pl --rmode=$rmode --cmode=$cmode $stem.raw | sort -u > $stem.dump.raw
#    if diff -q $stem.dump.raw $stem.dump.mci; then
#       echo "/ $rmode $cmode $i ok"
#    else
#       echo "/ $rmode $cmode $i error"
#       false
#    fi
# 
#    done
# 
#    $load -abc $stem.raw \
#       -$rmode-tab az.tab -write-tab $stem.dtab \
#       -o $stem.dci > /dev/null
#    mcxdump -imx $stem.dci -tab $stem.dtab --no-values | sort > $stem.dump.dci
#    $load -abc $stem.dump.dci -strict-tab $stem.dtab -o /dev/null > /dev/null
#    ./abc-pairs.pl --rmode=$rmode --cmode=$rmode $stem.raw | sort -u > $stem.dump.daw
#    if diff -q $stem.dump.daw $stem.dump.dci; then
#       echo "= $rmode $i ok"
#    else
#       echo "= $rmode $i error"
#       false
#    fi
# 
#    done
# done



