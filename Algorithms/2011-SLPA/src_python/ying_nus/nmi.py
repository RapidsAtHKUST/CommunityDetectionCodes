#!/usr/bin/python

'''
Created on Oct 12, 2013
I did not complete this class because I found 
previous implementation of the same method

@author: yiping
'''

import numpy as np
import getopt
import sys

def usage():
  print "\nThis is the usage function\n"
  print 'Usage: '+sys.argv[0]+' -i <our output file> -g <gold standard file> -n <num nodes>'

def main(argv):
    """normalized mutual information measure

    measures the similarity of two covers, output a float number in range [0, 1]

    parameters:
        -i: the location of the file to store the output of overlapping community detection
        -g: the location of the file containing the gold standard
        -n: number of nodes

    """

    try:
        opts, args = getopt.getopt(argv, 'hi:g:n:', ['help', 'input file=', 'gold standard file=', 'number of nodes='])
        if not opts or len(opts) != 3:
          print 'Wrong number of options supplied'
          usage()
          sys.exit(2)

        #get sys arguments
        input_file = opts[0][1]
        print "input file at %s" % input_file
        gold_standard_file = opts[1][1]
        print "gold standard file at %s" % gold_standard_file
        num_nodes = int(opts[2][1])
        print "graph contains %d nodes" % num_nodes

        #read the clusters in the two covers
        clusters1 = []

        f = open(input_file, "r")
        lines = f.readlines()
        for line in lines:
            if ":" in line:
                line = line.split(":")[-1]
            clusters1.append([int(node) for node in line.split(" ")])  #get the nodes that are separated by a space
            
        clusters2 = []

        f = open(gold_standard_file, "r")
        lines = f.readlines()
        for line in lines:
            if ":" in line:
                line = line.split(":")[-1]
            clusters2.append([int(node) for node in line.split(" ")])  #get the nodes that are separated by a space
        
        entropy1 = []  #vector with length k, entrophy of each cluster in clusters1
        entropy2 = []  #vector with length l, entrophy of each cluster in clusters2
        #NOTE: relative entropy is not symetric
        relative_entropy_xy = []  # 2D array, l*k, relative entropy H(X_k,y_l)
        relative_entropy_yx = []  # 2D array, k*l, relative entropy H(y_l,x_k)
 
        for cluster in clusters1:
            prob = len(cluster)/num_nodes  # probability of a random node belongs to this cluster
            entropy1.append(prob*) 

    except getopt.GetoptError,e:
        print e
        usage()
        sys.exit(2)        

# End of main().

if __name__ == "__main__":
    main(sys.argv[1:])
