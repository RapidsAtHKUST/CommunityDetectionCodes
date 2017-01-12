#!/usr/bin/env bash

# loop_thresholds.sh
# Jim Bagrow
# Last Modified: 2009-03-10


# Copyright 2009,2010 James Bagrow
# 
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.



# these need to be updated by you:
EXEC=./clusterJaccards
NET=karate.pairs
JACC=karate.jaccs
OUTDIR=clusters
# $EXEC network.pairs network.jaccs network.clusters network.cluster_stats threshold

for thr in 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1
do
    echo $thr
    $EXEC $NET $JACC $OUTDIR/network_$thr.cluster $OUTDIR/network_$thr.cluster_stats $thr
done
