#!/usr/bin/env python
# encoding:UTF-8

import numpy as np
import gc


def sort_two_vertices(a, b):
    if a > b:
        return b, a
    return a, b


def read_edgelist(filename, delimiter=None, nodetype=int):
    """Generate the adjacent matrix of a graph with overlapping communities.
       Input: A two-clumn edgelist
       Return: An adjacency matrix in the form of ndarray corresponding to the given edge list.
    """
    edgeset = set()
    nodeset = set()

    print "Reading the edgelist..."
    with open(filename) as f:
        for line in f.readlines():
            if not line.strip().startswith("#"):
                L = line.strip().split(delimiter)
                ni, nj = nodetype(L[0]), nodetype(L[1])
                nodeset.add(ni)
                nodeset.add(nj)
                if ni != nj:
                    edgeset.add(sort_two_vertices(ni, nj))
        node_number = len(list(nodeset))
        edge_number = len(list(edgeset))
    del edgeset

    print "The network has", node_number, "nodes with", edge_number, "edges."
    graph_linklist = [set() for i in range(0, node_number)]  # Initialize the graph in the format of linked list
    del nodeset

    for i in range(node_number):
        graph_linklist[i].add(i)
    with open(filename, 'U') as f:
        for line in f.readlines():
            if not line.strip().startswith("#"):
                L = line.strip().split(delimiter)
                ni, nj = nodetype(L[0]), nodetype(L[1])
                if ni != nj:
                    a = ni - 1
                    b = nj - 1
                    graph_linklist[a].add(b)
                    graph_linklist[b].add(a)
    gc.collect()

    degree = []
    for node in range(node_number):
        degree.append(len(graph_linklist[node]))

    print "Finish constructing graph."
    print "-------------------------------------------------------"

    return graph_linklist, node_number, edge_number, degree


def read_groundtruth(filename, delimiter=None, nodetype=int):
    comm_count = 12000
    print "Parsing ground truth communities..."
    communities = [[] for i in range(comm_count)]

    with open(filename, 'U') as f:
        count = 0
        for line in f.readlines():
            if not line.strip().startswith("#"):
                L = line.strip().split('\t')
                membership_array = np.fromstring(L[0], dtype=int, sep=' ')

                communities[count] = membership_array
                count += 1
    print "Finish parsing communities."

    return communities, count
