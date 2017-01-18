import collections
import random

from util_helper import *


class PPR:
    def __init__(self, graph, alpha, tolerance):
        self.graph = graph
        self.alpha = alpha
        self.tolerance = tolerance
        self.vol_of_graph = get_edge_num(graph)

    def generate_seed_list(self):
        rand_int = random.randint(1, len(self.graph))
        return [rand_int]

    def out_degree(self, v):
        return len(self.graph[v])

    def estimate_ppr_vector(self, seed_list):
        x_dict, r_dict = {}, {}
        task_queue = collections.deque()

        for vertex in seed_list:
            r_dict[vertex] = 1.0 / len(seed_list)
            task_queue.append(vertex)

        while len(task_queue) > 0:
            v = task_queue.popleft()
            x_dict[v] = 0 if v not in x_dict else x_dict[v] + (1 - self.alpha) * r_dict[v]
            mass = self.alpha * r_dict[v] / (2 * self.out_degree(v))

            for neighbor_v in self.graph[v]:
                r_dict[neighbor_v] = 0.0 if neighbor_v not in r_dict else r_dict[neighbor_v] + mass
                if r_dict[neighbor_v] - mass < self.out_degree(v) * self.tolerance <= r_dict[neighbor_v]:
                    task_queue.append(neighbor_v)

            r_dict[v] *= self.alpha / 2
            if r_dict[v] >= self.out_degree(v) * self.tolerance:
                task_queue.append(v)
        print str(x_dict)
        return x_dict

    def sweep_cut(self, x_dict):
        vertex_weight_list = sorted(map(lambda ele: (ele[0], ele[1] / len(self.graph[ele[0]])), x_dict.iteritems()),
                                    key=lambda x: x[1], reverse=True)

        candidate_set = set()
        vol_of_set, cut_of_set = 0.0, 0.0
        best_cond, best_set = 1.0, vertex_weight_list[0]
        for vertex, weight in vertex_weight_list:
            vol_of_set += self.out_degree(vertex)
            cut_of_set += sum(map(lambda neighbor_v: -1 if neighbor_v in candidate_set else 1, self.graph[vertex]))
            candidate_set.add(vertex)
            tmp_cond = compute_conductance(cut_of_set, vol_of_set, self.vol_of_graph)
            if tmp_cond < best_cond:
                best_cond = tmp_cond
                best_set = set(candidate_set)
        return best_set, best_cond

    def do_iterations(self):
        x_dict = self.estimate_ppr_vector(self.generate_seed_list())
        best_set, best_cond = self.sweep_cut(x_dict)
        print "Best set conductance: %f" % (best_cond)
        print "  set = ", str(best_set)


if __name__ == '__main__':
    adj_list_dict = get_sample_graph()
    PPR(graph=adj_list_dict, alpha=0.99, tolerance=0.01).do_iterations()
