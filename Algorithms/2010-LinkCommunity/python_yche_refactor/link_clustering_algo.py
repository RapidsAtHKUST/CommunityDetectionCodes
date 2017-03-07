#!/usr/bin/env python
# encoding: utf-8

from copy import copy
from heapq import heappush, heappop
from itertools import combinations, chain


def get_sorted_pair(a, b):
    return b, a if a > b else a, b


def sort_edge_pairs_by_similarity(adj_list_dict):
    def cal_jaccard_non_weighted(left_set, right_set):
        return 1.0 * len(left_set & right_set) / len(left_set | right_set)

    inc_adj_list_dict = dict((n, adj_list_dict[n] | {n}) for n in adj_list_dict)
    min_heap = []
    for vertex in adj_list_dict:
        if len(adj_list_dict[vertex]) > 1:
            for i, j in combinations(adj_list_dict[vertex], 2):
                edge_pair = get_sorted_pair(get_sorted_pair(i, vertex), get_sorted_pair(j, vertex))
                inc_neighbors_i, inc_neighbors_j = inc_adj_list_dict[i], inc_adj_list_dict[j]
                similarity_ratio = cal_jaccard_non_weighted(inc_neighbors_i, inc_neighbors_j)
                heappush(min_heap, (1 - similarity_ratio, edge_pair))
    return [heappop(min_heap) for _ in xrange(len(min_heap))]


def sort_edge_pairs_by_similarity_weighted(adj_dict, edge_weight_dict):
    inc_adj_list_dict = dict((n, adj_dict[n] | {n}) for n in adj_dict)

    def cal_jaccard_weighted(intersect_val, left_val, right_val):
        return intersect_val / (left_val, right_val - intersect_val)

    Aij = copy(edge_weight_dict)
    n2a_sqrd = {}
    for vertex in adj_dict:
        Aij[vertex, vertex] = float(sum(edge_weight_dict[get_sorted_pair(vertex, i)] for i in adj_dict[vertex]))
        Aij[vertex, vertex] /= len(adj_dict[vertex])
        n2a_sqrd[vertex] = sum(Aij[get_sorted_pair(vertex, i)] ** 2
                               for i in inc_adj_list_dict[vertex])  # includes (n,n)!

    min_heap = []
    for vertex in adj_dict:
        if len(adj_dict[vertex]) > 1:
            for i, j in combinations(adj_dict[vertex], 2):
                edge_pair = get_sorted_pair(get_sorted_pair(i, vertex), get_sorted_pair(j, vertex))
                inc_ns_i, inc_ns_j = inc_adj_list_dict[i], inc_adj_list_dict[j]

                ai_dot_aj = float(
                    sum(Aij[get_sorted_pair(i, x)] * Aij[get_sorted_pair(j, x)] for x in inc_ns_i & inc_ns_j))

                similarity_ratio = cal_jaccard_weighted(ai_dot_aj, n2a_sqrd[i], n2a_sqrd[j])
                heappush(min_heap, (1 - similarity_ratio, edge_pair))
    return [heappop(min_heap) for _ in xrange(len(min_heap))]


# Hierarchical Link Community
class HLC:
    def __init__(self, adj_list_dict, edges):
        self.adj_list_dict = adj_list_dict
        self.edges = edges
        self.density_factor = 2.0 / len(edges)

        self.edge2cid = {}
        self.cid2nodes, self.cid2edges = {}, {}
        self.orig_cid2edge = {}
        self.curr_max_cid = 0
        self.linkage = []  # dendrogram

        self.D = 0.0  # partition density

        def initialize_edges():
            for cid, edge in enumerate(self.edges):
                edge = get_sorted_pair(*edge)  # just in case
                self.edge2cid[edge] = cid
                self.cid2edges[cid] = {edge}
                self.orig_cid2edge[cid] = edge
                self.cid2nodes[cid] = set(edge)
            self.curr_max_cid = len(self.edges) - 1

        initialize_edges()  # every edge in its own comm

        self.list_D = [(1.0, 0.0)]  # list of (S_i,D_i) tuples...
        self.best_D = 0.0
        self.best_S = 1.0  # similarity threshold at self.best_D
        self.best_P = None  # best partition, dict: edge -> cid

    def merge_comms(self, edge1, edge2, S, dendro_flag=False):
        def cal_density(edge_num, vertex_num):
            return edge_num * (edge_num - vertex_num + 1.0) / (
                (vertex_num - 2.0) * (vertex_num - 1.0)) if vertex_num > 2 else 0.0

        if not edge1 or not edge2:  # We'll get (None, None) at the end of clustering
            return
        cid1, cid2 = self.edge2cid[edge1], self.edge2cid[edge2]
        if cid1 == cid2:  # already merged!
            return
        m1, m2 = len(self.cid2edges[cid1]), len(self.cid2edges[cid2])
        n1, n2 = len(self.cid2nodes[cid1]), len(self.cid2nodes[cid2])
        Dc1, Dc2 = cal_density(m1, n1), cal_density(m2, n2)
        if m2 > m1:  # merge smaller into larger
            cid1, cid2 = cid2, cid1

        if dendro_flag:
            self.curr_max_cid += 1
            newcid = self.curr_max_cid
            self.cid2edges[newcid] = self.cid2edges[cid1] | self.cid2edges[cid2]
            self.cid2nodes[newcid] = set()
            for e in chain(self.cid2edges[cid1], self.cid2edges[cid2]):
                self.cid2nodes[newcid] |= set(e)
                self.edge2cid[e] = newcid
            del self.cid2edges[cid1], self.cid2nodes[cid1]
            del self.cid2edges[cid2], self.cid2nodes[cid2]
            m, n = len(self.cid2edges[newcid]), len(self.cid2nodes[newcid])

            self.linkage.append((cid1, cid2, S))

        else:
            self.cid2edges[cid1] |= self.cid2edges[cid2]
            for e in self.cid2edges[cid2]:  # move edges,nodes from cid2 to cid1
                self.cid2nodes[cid1] |= set(e)
                self.edge2cid[e] = cid1
            del self.cid2edges[cid2], self.cid2nodes[cid2]

            m, n = len(self.cid2edges[cid1]), len(self.cid2nodes[cid1])

        Dc12 = cal_density(m, n)
        self.D += (Dc12 - Dc1 - Dc2) * self.density_factor  # update partition density

    def single_linkage(self, threshold=None, w=None, dendro_flag=False):
        if w is None:
            H = sort_edge_pairs_by_similarity(self.adj_list_dict)  # min-heap ordered by 1-s
        else:
            H = sort_edge_pairs_by_similarity_weighted(self.adj_list_dict, w)
        prev_similarity = -1

        # (1.0, (None, None)) takes care of the special case where the last
        # merging gives the maximum partition density (e.g. a single clique).
        for oms, eij_eik in chain(H, [(1.0, (None, None))]):
            cur_similarity = 1 - oms
            if threshold and cur_similarity < threshold:
                break

            if cur_similarity != prev_similarity:  # update list
                if self.D >= self.best_D:  # check PREVIOUS merger, because that's
                    self.best_D = self.D  # the end of the tie
                    self.best_S = cur_similarity
                    self.best_P = copy(self.edge2cid)  # slow...
                self.list_D.append((cur_similarity, self.D))
                prev_similarity = cur_similarity
            self.merge_comms(eij_eik[0], eij_eik[1], cur_similarity, dendro_flag)

        # self.list_D.append( (0.0,self.list_D[-1][1]) ) # add final val
        if threshold is not None:
            return self.edge2cid, self.D
        if dendro_flag:
            return self.best_P, self.best_S, self.best_D, self.list_D, self.orig_cid2edge, self.linkage
        else:
            return self.best_P, self.best_S, self.best_D, self.list_D
