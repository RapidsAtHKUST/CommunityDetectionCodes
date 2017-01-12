#!/usr/bin/env python
# encoding: utf-8

import os
from optparse import OptionParser
from graph_io_helper import *
from link_clustering_algo import *


class MyParser(OptionParser):
    def format_epilog(self, formatter):
        return self.epilog


if __name__ == '__main__':
    usage = "usage: python %prog [options] filename"
    parser = MyParser(usage)
    parser.add_option("-d", "--delimiter", dest="delimiter", default="\t",
                      help="delimiter of input & output files [default: tab]")
    parser.add_option("-t", "--threshold", dest="threshold", type="float", default=None,
                      help="threshold to cut the dendrogram (optional)")
    parser.add_option("-w", "--weighted", dest="is_weighted", action="store_true", default=False,
                      help="is the network weighted?")
    parser.add_option("-r", "--record-dendrogram", dest="dendro_flag", action="store_true",
                      default=False, help="recording the whole dendrogram (optional)")

    (options, args) = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")
    delimiter = options.delimiter
    if delimiter == '\\t':
        delimiter = '\t'
    threshold = options.threshold
    is_weighted = options.is_weighted
    dendro_flag = options.dendro_flag

    print "# loading network from edgelist..."
    basename = os.path.splitext(args[0])[0]
    if is_weighted:
        adj, edges, ij2wij = read_edge_list_weighted(args[0], delimiter=delimiter)
    else:
        adj, edges = read_edge_list_unweighted(args[0], delimiter=delimiter)

    if threshold is not None:
        if is_weighted:
            edge2cid, D_thr = HLC(adj, edges).single_linkage(threshold, w=ij2wij)
        else:
            edge2cid, D_thr = HLC(adj, edges).single_linkage(threshold)
        print "# D_thr = %f" % D_thr
        write_edge2cid(edge2cid, "%s_thrS%f_thrD%f" % (basename, threshold, D_thr), delimiter=delimiter)
    else:
        if is_weighted:
            edge2cid, S_max, D_max, list_D = HLC(adj, edges).single_linkage(w=ij2wij)
        else:
            if dendro_flag:
                edge2cid, S_max, D_max, list_D, orig_cid2edge, linkage = HLC(adj, edges).single_linkage(
                    dendro_flag=dendro_flag)
                write_dendro("%s_dendro" % basename, orig_cid2edge, linkage)
            else:
                edge2cid, S_max, D_max, list_D = HLC(adj, edges).single_linkage()

        f = open("%s_thr_D.txt" % basename, 'w')
        for s, D in list_D:
            print >> f, s, D
        f.close()
        print "# D_max = %f\n# S_max = %f" % (D_max, S_max)
        write_edge2cid(edge2cid, "%s_maxS%f_maxD%f" % (basename, S_max, D_max), delimiter=delimiter)
