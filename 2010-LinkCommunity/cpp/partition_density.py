#!/usr/bin/env python
# encoding: utf-8

# partition_density.py
# Jim Bagrow
# Last Modified: 2010-02-16

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



import sys, os
import glob

def load_cluster_stats(filename):
    return [map(int, line.strip().split()) for line in open(filename)]


def partition_density(list_mc_nc):
    M, wSum = 0, 0.0
    for mc,nc in list_mc_nc:
        M += mc    # penalize singletons?
        if nc != 2: 
            wSum += mc * (mc - (nc-1.0)) / ((nc-2.0)*(nc-1.0))
    return  2.0 * wSum / M


def partition_density_from_file(filename):
    M, wSum = 0, 0.0
    for line in open(filename):
        mc, nc = map(int, line.strip().split())
        M += mc
        if nc != 2: 
            wSum += mc * (mc - (nc-1.0)) / ((nc-2.0)*(nc-1.0))
    return 2.0 * wSum / M


if __name__ == '__main__':
    try:
        f = sys.argv[1]
    except:
        sys.exit( "usage: %s network.mc_nc" % sys.argv[0] )
    
    print "D =", partition_density_from_file(f)
    
    """
    fout = open("unweighted_thr_PD.mat.txt", 'w')
    files = reversed(sorted(glob.glob("unweighted/clusters/*cluster_stats")))
    for f in files:
        thr = float( ".".join(f.split(".")[0:2]).split("_")[1] ) # ugly!
        D = partition_density_from_file(f)
        print thr, D
        print >>fout, thr, D
    fout.close()
    
    print
    try:
        fout = open("weighted_thr_PD.mat.txt", 'w')
        files = reversed(sorted(glob.glob("weighted/clusters/*cluster_stats")))
        for f in files:
            thr = float( ".".join(f.split(".")[0:2]).split("_")[1] ) # ugly!
            D = partition_density_from_file(f)
            print thr, D
            print >>fout, thr, D
        fout.close()
    except:
        pass
    """