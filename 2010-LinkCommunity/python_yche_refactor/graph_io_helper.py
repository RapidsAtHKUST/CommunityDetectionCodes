#!/usr/bin/env python
# encoding: utf-8


from operator import itemgetter
from collections import defaultdict


def sort_two_vertices(a, b):
    if a > b:
        return b, a
    return a, b


def read_edgelist_unweighted(filename, delimiter=None, nodetype=str):
    """reads two-column edgelist, returns dictionary
    mapping node -> set of neighbors and a list of edges
    """
    adj = defaultdict(set)  # node to set of neighbors
    edges = set()
    for line in open(filename, 'U'):
        L = line.strip().split(delimiter)
        ni, nj = nodetype(L[0]), nodetype(L[1])  # other columns ignored
        if ni != nj:  # skip any self-loops...
            edges.add(sort_two_vertices(ni, nj))
            adj[ni].add(nj)
            adj[nj].add(ni)  # since undirected
    return dict(adj), edges


def read_edgelist_weighted(filename, delimiter=None, nodetype=str, weighttype=float):
    """same as read_edgelist_unweighted except the input file now has three
    columns: node_i<delimiter>node_j<delimiter>weight_ij<newline>
    and the output includes a dict `ij2wij' mapping edge tuple (i,j) to w_ij
    """
    adj = defaultdict(set)
    edges = set()
    ij2wij = {}
    for line in open(filename, 'U'):
        L = line.strip().split(delimiter)
        ni, nj, wij = nodetype(L[0]), nodetype(L[1]), weighttype(L[2])  # other columns ignored
        if ni != nj:  # skip any self-loops...
            ni, nj = sort_two_vertices(ni, nj)
            edges.add((ni, nj))
            ij2wij[ni, nj] = wij
            adj[ni].add(nj)
            adj[nj].add(ni)  # since undirected
    return dict(adj), edges, ij2wij


def write_edge2cid(e2c, filename, delimiter="\t"):
    # renumber community id's to be sequential, makes output file human-readable
    c2c = dict((c, i + 1) for i, c in enumerate(sorted(list(set(e2c.values())))))  # ugly...

    f = open(filename + ".edge2comm.txt", 'w')
    for e, c in sorted(e2c.iteritems(), key=itemgetter(1)):
        f.write("%s%s%s%s%s\n" % (str(e[0]), delimiter, str(e[1]), delimiter, str(c2c[c])))
    f.close()

    cid2edges, cid2nodes = defaultdict(set), defaultdict(set)  # faster to recreate here than
    for edge, cid in e2c.iteritems():  # to keep copying all dicts
        cid2edges[cid].add(edge)  # during the linkage...
        cid2nodes[cid] |= set(edge)
    cid2edges, cid2nodes = dict(cid2edges), dict(cid2nodes)

    f, g = open(filename + ".comm2edges.txt", 'w'), open(filename + ".comm2nodes.txt", 'w')
    for cid in sorted(cid2edges.keys()):
        strcid = str(c2c[cid])
        nodes = map(str, cid2nodes[cid])
        edges = ["%s,%s" % (ni, nj) for ni, nj in cid2edges[cid]]
        f.write(delimiter.join([strcid] + edges))
        f.write("\n")
        g.write(delimiter.join([strcid] + nodes))
        g.write("\n")
    f.close()
    g.close()


def write_dendro(filename, orig_cid2edge, linkage):
    with open(filename + '.cid2edge.txt', 'w') as fout:
        for cid, e in orig_cid2edge.iteritems():
            fout.write("%d\t%s,%s\n" % (cid, str(e[0]), str(e[1])))

    with open(filename + '.linkage.txt', 'w') as fout:
        for x in linkage:
            fout.write('%s\n' % '\t'.join(map(str, x)))
