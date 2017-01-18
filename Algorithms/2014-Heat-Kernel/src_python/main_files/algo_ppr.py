import collections
import random

from util_helper import *


class PPR:
    def __init__(self, graph, alpha, tolerance):
        self.graph = graph
        self.alpha = alpha
        self.tolerance = tolerance
        self.volG = get_edge_num(graph)

    def generate_seed_list(self):
        rand_int = random.randint(1, len(self.graph))
        return [rand_int]

    def estimate_ppr_vector(self, seed_list):
        x_dict, r_dict = {}, {}
        task_queue = collections.deque()

        for vertex in seed_list:
            r_dict[vertex] = 1.0 / len(seed_list)
            task_queue.append(vertex)

        while len(task_queue) > 0:
            v = task_queue.popleft()
            if v not in x_dict:
                x_dict[v] = 0.
            x_dict[v] += (1 - self.alpha) * r_dict[v]
            mass = self.alpha * r_dict[v] / (2 * len(self.graph[v]))

            for neighbor_v in self.graph[v]:
                if neighbor_v not in r_dict:
                    r_dict[neighbor_v] = 0.
                if r_dict[neighbor_v] < len(self.graph[neighbor_v]) * self.tolerance <= r_dict[neighbor_v] + mass:
                    task_queue.append(neighbor_v)
                r_dict[neighbor_v] += mass
            r_dict[v] = mass * len(self.graph[v])
            if r_dict[v] >= len(self.graph[v]) * self.tolerance:
                task_queue.append(v)
        print str(x_dict)
        return x_dict

    def sweep_cut(self, x_dict):
        sorted_pair = sorted(map(lambda ele: (ele[0], ele[1] / len(self.graph[ele[0]])), x_dict.iteritems()),
                             key=lambda x: x[1], reverse=True)
        S = set()
        volS, cutS = 0., 0.
        best_cond = 1.
        best_set = sorted_pair[0]
        for p in sorted_pair:
            vertex = p[0]  # get the vertex
            volS += len(self.graph[vertex])  # add degree to volume
            cutS += sum(map(lambda v: -1 if v in S else 1, self.graph[vertex]))
            S.add(vertex)
            if cutS / min(volS, self.volG - volS) < best_cond:
                best_cond = cutS / min(volS, self.volG - volS)
                best_set = set(S)  # make a copy
        return best_cond, best_set

    def do_iterations(self):
        x_dict = self.estimate_ppr_vector(self.generate_seed_list())
        best_cond, best_set = self.sweep_cut(x_dict)
        print "Best set conductance: %f" % (best_cond)
        print "  set = ", str(best_set)


if __name__ == '__main__':
    adj_list_dict = get_sample_graph()
    PPR(graph=adj_list_dict, alpha=0.99, tolerance=0.01).do_iterations()
