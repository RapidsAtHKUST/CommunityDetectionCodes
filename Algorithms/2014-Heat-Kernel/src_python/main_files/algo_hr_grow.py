# Heat Kernel Based Community Detection
# by Kyle Kloster and David F. Gleich
# supported by NSF award CCF-1149756.
# refactored by Yulin CHE

import collections
import random

from util_helper import *


class HRGrow:
    def __init__(self, N, t, eps, svg_graph):
        self.N = N
        self.t = t
        self.eps = eps
        self.psis = compute_psis(N, t)
        self.graph = svg_graph
        self.volG = self.graph.nedges

    def estimate_hkpr_vector(self, seed_list):
        x_dict = dict()
        residual_dict = dict()
        task_queue = collections.deque()  # initialize queue

        for s in seed_list:
            residual_dict[(s, 0)] = 1. / len(seed_list)
            task_queue.append((s, 0))
        push_num = float(len(seed_list))

        while len(task_queue) > 0:
            (v, j) = task_queue.popleft()  # v has r[(v,j)] ...
            rvj = residual_dict[(v, j)]

            # perform the hk-relax step
            if v not in x_dict:
                x_dict[v] = 0.
            x_dict[v] += rvj
            residual_dict[(v, j)] = 0.
            update = rvj / self.graph.out_degree(v)
            mass = (self.t / (float(j) + 1.)) * update

            # for neighbors of v
            for u in self.graph[v]:
                next_block = (u, j + 1)
                if j + 1 == self.N:
                    x_dict[u] += update
                else:
                    if next_block not in residual_dict:
                        residual_dict[next_block] = 0.

                    thresh = math.exp(self.t) * self.eps * self.graph.out_degree(u)
                    thresh /= self.N * self.psis[j + 1]
                    if residual_dict[next_block] < thresh <= residual_dict[next_block] + mass:
                        task_queue.append(next_block)  # add u to queue
                    residual_dict[next_block] += mass
            push_num += self.graph.out_degree(v)
        return x_dict, push_num

    def sweep_cut(self, x_dict, seed_list, push_num, start):
        for v in x_dict:
            x_dict[v] = x_dict[v] / self.graph.out_degree(v)
        sv = sorted(x_dict.iteritems(), key=lambda x: x[1], reverse=True)

        S = set()
        volS = 0.
        cutS = 0.
        best_cond = 1.
        best_set = sv[0]
        for p in sv:
            s = p[0]  # get the vertex
            volS += self.graph.out_degree(s)  # add degree to volume
            for v in self.graph[s]:
                if v in S:
                    cutS -= 1
                else:
                    cutS += 1
            S.add(s)
            if cutS / min(volS, self.volG - volS) < best_cond:
                best_cond = cutS / min(volS, self.volG - volS)
                best_set = set(S)  # make a copy

        print "%10i  %5i  %4.2f  %4.2f  %7i  %7i  %7i" % (
            seed_list[0], self.graph.out_degree(seed_list[0]), time.time() - start, best_cond, push_num, len(x_dict),
            len(best_set))

    def generate_seed_list(self):
        rand_int = random.randint(1, len(self.graph))
        return [rand_int]

    def do_iterations(self):
        iter_round = 0
        while True:
            if iter_round % 15 == 0:
                print "%10s  %5s  %4s  %4s  %7s  %7s  %7s" % (
                    'seed ID', 'degree', 'time', 'cond', 'edges', 'nnz', 'setsize')
            iter_round += 1
            time.sleep(0.5)

            seed_list = self.generate_seed_list()
            start = time.time()

            # Step 1: Estimate hkpr vector
            x_dict, push_num = self.estimate_hkpr_vector(seed_list)
            # Step 2: do a sweep cut based on this vector
            self.sweep_cut(x_dict, seed_list, push_num, start)


if __name__ == '__main__':
    twitter_graph = load_twitter_graph()

    # Setup parameters that can be computed automatically
    HRGrow(N=47, t=15, eps=0.0001, svg_graph=twitter_graph).do_iterations()
