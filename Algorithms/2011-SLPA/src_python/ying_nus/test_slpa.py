#!/usr/bin/python

'''
Created on Oct 12, 2013

@author: yiping
'''

import numpy as np
#import slpa
import time
import random 
import getopt
import sys

def usage():
  print "\nThis is the usage function\n"
  print 'Usage: '+sys.argv[0]+' -i <file> -n <num nodes>'

def main(argv):
    """test slpa, randomly generate a graph

    Attributes:
        FILE_LOCATION: the location of the file to store the graph
        N: number of nodes in the graph
        LAMDA: I assume poisson distribution of the degrees of nodes.
               This is the lamda variable of the distribution        

        iteration: the number of iterations to run slpa
        threshhold: r => [0,1], if the probability is less than r,
                    remove it during post processing

    """
    FILE_LOCATION = 'input_graph.txt'
    N = 5000  #total 1,000,000 nodes
    LAMDA = 10  #average degree set to 200

    ITERATION = 20
    THRESHHOLD = 0.1

    #perform poisson sampling
    start_time = time.time()
    
    f = open(FILE_LOCATION,"w+")

    f.write("%d\n" % N)  #write the number of nodes in first line
    f.write("%d\n" % LAMDA) #write lamda in second line

    degrees = np.random.poisson(LAMDA, N)

    for i in range(N):
        #generate an array of size degrees[i] of random integers
        if degrees[i] > N:
            degrees[i] = N 
        #neighbors = numpy.random.randint(N,size=degrees[i])
        neighbors = random.sample(range(N), degrees[i])

        if i in neighbors:
            neighbors.remove(i)

        f.write("%s\n" % ' '.join(str(neighbor) for neighbor in neighbors))
    # End of loop

    f.close()

    end_time = time.time()
    print("Elapsed time to generate directed graph %g seconds" % (end_time - start_time))

    try:
        opts, args = getopt.getopt(argv, 'hi:n:', ['help', 'input file=', 'number of nodes='])
        if not opts or len(opts) != 2:
          print 'Wrong number of options supplied'
          usage()
          sys.exit(2)

        #get sys arguments
        input_file = opts[0][1]
        print "input file at %s" % input_file
        num_nodes = int(opts[1][1])
        print "graph contains %d nodes" % num_nodes

        # generate undirected graph
        adjacency_list = []
        #initialize adjacency_list with empty list
        for i in range(num_nodes):
            adjacency_list.append([])

        f = open(input_file,"r")
        lines = f.readlines()
  
        for line in lines: 
            node1, node2 = [int(part) for part in line.split("\t")]
            node1 -= 1
            node2 -= 1
            adjacency_list[node1].append(node2)
            adjacency_list[node2].append(node1)
     
        f = open("input_graph_undirected.txt","w+")
        f.write("%d\n" % num_nodes)  #write the number of nodes in first line
        f.write("%d\n" % LAMDA) #write lamda in second line

        for node in adjacency_list:
            f.write("%s\n" % " ".join([str(neighbor) for neighbor in node]))

        f.close()

    except getopt.GetoptError,e:
        print e
        usage()
        sys.exit(2)
    # end of try except

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage()
            sys.exit(2)

    '''
    for i in range(N):
        adjacency_list.append([np.random.randint(N)])

    start_time = time.time()

    for i in range(N*LAMDA/2):
        edge = random.sample(range(N), 2)
        adjacency_list[edge[0]].append(edge[1])
        adjacency_list[edge[1]].append(edge[0])

    end_time = time.time()
    print("Elapsed time to generate undirected graph %g seconds" % (end_time - start_time))
    '''
    

# End of main().

if __name__ == "__main__":
    main(sys.argv[1:])
