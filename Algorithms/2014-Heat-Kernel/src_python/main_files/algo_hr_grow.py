# Heat Kernel Based Community Detection
# by Kyle Kloster and David F. Gleich
# supported by NSF award CCF-1149756.
# refactored by Yulin CHE

import collections
import random

from util_helper import *


class HRGrow:
    @staticmethod
    def compute_conductance(cut_num, vol_s, vol_g):
        return cut_num / min(vol_s, vol_g - vol_s)

    def __init__(self, N, t, eps, svg_graph):
        self.N = N
        self.t = t
        self.eps = eps
        self.psis = compute_psis(N, t)
        self.graph = svg_graph
        self.vol_of_graph = self.graph.nedges

    def generate_seed_list(self):
        rand_int = random.randint(1, len(self.graph))
        return [rand_int]

    def estimate_hkpr_vector(self, seed_list):
        x_dict, r_dict = dict(), dict()
        task_queue, iter_num = collections.deque(), 0
        for s in seed_list:
            r_dict[(s, iter_num)] = 1. / len(seed_list)
            task_queue.append((s, iter_num))

        push_num = len(seed_list)

        while len(task_queue) > 0:
            v, iter_num = task_queue.popleft()  # v has r[(v,j)] ...
            r_weight = r_dict[(v, iter_num)]

            # perform the hk-relax step
            if v not in x_dict:
                x_dict[v] = 0.

            x_dict[v] += r_weight
            r_dict[v, iter_num] = 0.
            update = r_weight / self.graph.out_degree(v)
            mass = (self.t / (float(iter_num) + 1.)) * update

            # for neighbors of v
            for neighbor_v in self.graph[v]:
                potential_task = (neighbor_v, iter_num + 1)
                if iter_num + 1 == self.N:
                    x_dict[neighbor_v] += update
                else:
                    if potential_task not in r_dict:
                        r_dict[potential_task] = 0.

                    thresh = math.exp(self.t) * self.eps * self.graph.out_degree(neighbor_v) \
                             / (self.N * self.psis[iter_num + 1])

                    if r_dict[potential_task] < thresh <= r_dict[potential_task] + mass:
                        task_queue.append(potential_task)  # add u to queue
                    r_dict[potential_task] += mass
            push_num += self.graph.out_degree(v)
        return x_dict, push_num

    def sweep_cut(self, x_dict):
        vertex_weight_list = sorted(
            map(lambda ele: (ele[0], ele[1] / self.graph.out_degree(ele[0])), x_dict.iteritems()),
            key=lambda x: x[1], reverse=True)

        candidate_set = set()
        vol_of_set, cut_of_set = 0.0, 0.0
        best_cond, best_set = 1.0, vertex_weight_list[0]
        for vertex, weight in vertex_weight_list:
            vol_of_set += self.graph.out_degree(vertex)
            cut_of_set += sum(map(lambda neighbor_v: -1 if neighbor_v in candidate_set else 1, self.graph[vertex]))
            candidate_set.add(vertex)
            if HRGrow.compute_conductance(cut_of_set, vol_of_set, self.vol_of_graph) < best_cond:
                best_cond = cut_of_set / min(vol_of_set, self.vol_of_graph - vol_of_set)
                best_set = set(candidate_set)
        return best_set, best_cond

    def do_iterations(self):
        iter_round = 0
        while True:
            if iter_round % 15 == 0:
                print "%10s  %5s  %4s  %4s  %7s  %7s  %7s" % (
                    'seed ID', 'degree', 'time', 'cond', 'edges', 'nnz', 'setsize')
            iter_round += 1
            time.sleep(0.5)
            start = time.time()

            seed_list = self.generate_seed_list()
            x_dict, push_num = self.estimate_hkpr_vector(seed_list)
            best_set, best_cond = self.sweep_cut(x_dict)

            print "%10i  %5i  %4.2f  %4.2f  %7i  %7i  %7i" % (
                seed_list[0], self.graph.out_degree(seed_list[0]), time.time() - start, best_cond, push_num,
                len(x_dict), len(best_set))


if __name__ == '__main__':
    twitter_graph = load_twitter_graph()
    HRGrow(N=47, t=15, eps=0.0001, svg_graph=twitter_graph).do_iterations()
