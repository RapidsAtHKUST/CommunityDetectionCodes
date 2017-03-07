#!/usr/bin/env python
# encoding: utf-8


from operator import itemgetter
from collections import defaultdict


def get_sorted_edge(a, b):
    return tuple(sorted([a, b]))


def read_edge_list_unweighted(filename, node_type=str):
    adj_list_dict = defaultdict(set)
    edges = set()
    with open(filename) as ifs:
        edge_list = map(lambda ele: ele.strip().split(), ifs.readlines())
        edge_list = filter(lambda edge: edge[0] != edge[1], map(lambda edge: map(node_type, edge), edge_list))
        print edge_list

        for ni, nj in edge_list:
            print ni, nj
            edges.add(get_sorted_edge(ni, nj))
            print edges
            adj_list_dict[ni].add(nj)
            adj_list_dict[nj].add(ni)
        print adj_list_dict
        print edges
        return dict(adj_list_dict), edges


def read_edge_list_weighted(filename, node_type=str, weight_type=float):
    adj = defaultdict(set)
    edges = set()
    ij2wij = {}
    for line in open(filename, 'U'):
        L = line.strip().split()
        ni, nj, wij = node_type(L[0]), node_type(L[1]), weight_type(L[2])
        if ni != nj:
            ni, nj = get_sorted_edge(ni, nj)
            edges.add((ni, nj))
            ij2wij[ni, nj] = wij
            adj[ni].add(nj)
            adj[nj].add(ni)
    return dict(adj), edges, ij2wij


def write_edge2cid(e2c, filename, delimiter="\t"):
    # renumber community id's to be sequential, makes output file human-readable
    c2c = dict((c, i + 1) for i, c in enumerate(sorted(list(set(e2c.values())))))  # ugly...

    with open(filename + ".edge2comm.txt", 'w') as f:
        for edge, coom in sorted(e2c.iteritems(), key=itemgetter(1)):
            f.write(delimiter.join(map(str, [edge[0], edge[1], c2c[coom]])) + '\n')

    cid2edges, cid2nodes = defaultdict(set), defaultdict(set)  # faster to recreate here than
    for edge, cid in e2c.iteritems():  # to keep copying all dicts
        cid2edges[cid].add(edge)  # during the linkage...
        cid2nodes[cid] |= set(edge)
    cid2edges, cid2nodes = dict(cid2edges), dict(cid2nodes)

    with open(filename + ".comm2edges.txt", 'w') as f:
        with open(filename + ".comm2nodes.txt", 'w') as g:
            for cid in sorted(cid2edges.keys()):
                strcid = str(c2c[cid])
                nodes = map(str, cid2nodes[cid])
                edges = ["%s,%s" % (ni, nj) for ni, nj in cid2edges[cid]]
                f.write(delimiter.join([strcid] + edges))
                f.write("\n")
                g.write(delimiter.join([strcid] + nodes))
                g.write("\n")


def write_dendro(filename, orig_cid2edge, linkage):
    with open(filename + '.cid2edge.txt', 'w') as fout:
        for cid, e in orig_cid2edge.iteritems():
            fout.write("%d\t%s,%s\n" % (cid, str(e[0]), str(e[1])))

    with open(filename + '.linkage.txt', 'w') as fout:
        for x in linkage:
            fout.write('%s\n' % '\t'.join(map(str, x)))
