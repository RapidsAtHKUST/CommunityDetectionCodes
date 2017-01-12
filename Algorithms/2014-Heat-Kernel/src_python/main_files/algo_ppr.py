import collections

from util_helper import *


class PPR:
    def __init__(self, graph, alpha, tol):
        self.graph = graph
        self.alpha = alpha
        self.tol = tol
        self.volG = get_edge_num(graph)

    @staticmethod
    def generate_seed_list():
        return [1]

    def estimate_ppr_vector(self, seed_list):
        x_dict = {}
        residual_dict = {}
        task_queue = collections.deque()

        seed_list = PPR.generate_seed_list()
        for vertex in seed_list:
            residual_dict[vertex] = 1.0 / len(seed_list)
            task_queue.append(vertex)

        # Personalized PageRank
        while len(task_queue) > 0:
            v = task_queue.popleft()  # v has r[v] > tol*deg(v)
            if v not in x_dict:
                x_dict[v] = 0.
            x_dict[v] += (1 - self.alpha) * residual_dict[v]
            mass = self.alpha * residual_dict[v] / (2 * len(self.graph[v]))
            for u in self.graph[v]:  # for neighbors of u
                assert u is not v, "contact dgleich@purdue.edu for self-links"
                if u not in residual_dict:
                    residual_dict[u] = 0.
                if residual_dict[u] < len(self.graph[u]) * self.tol <= residual_dict[u] + mass:
                    task_queue.append(u)  # add u to queue if large
                residual_dict[u] += mass
            residual_dict[v] = mass * len(self.graph[v])
            if residual_dict[v] >= len(self.graph[v]) * self.tol:
                task_queue.append(v)
        print str(x_dict)
        return x_dict

    def sweep_cut(self, x_dict):
        # Find cluster
        for v in x_dict:
            x_dict[v] /= len(self.graph[v])
        sorted_pair = sorted(x_dict.iteritems(), key=lambda x: x[1], reverse=True)

        S = set()
        volS, cutS = 0., 0.
        bestcond = 1.
        bestset = sorted_pair[0]
        for p in sorted_pair:
            vertex = p[0]  # get the vertex
            volS += len(self.graph[vertex])  # add degree to volume
            for v in self.graph[vertex]:
                if v in S:
                    cutS -= 1
                else:
                    cutS += 1
            print "v: %4i  cut: %4f  vol: %4f" % (vertex, cutS, volS)
            S.add(vertex)
            if cutS / min(volS, self.volG - volS) < bestcond:
                bestcond = cutS / min(volS, self.volG - volS)
                bestset = set(S)  # make a copy
        print "Best set conductance: %f" % (bestcond)
        print "  set = ", str(bestset)

    def do_iterations(self):
        x_dict = self.estimate_ppr_vector(self.generate_seed_list())
        self.sweep_cut(x_dict)


if __name__ == '__main__':
    adj_list_dict = get_sample_graph()
    PPR(graph=adj_list_dict, alpha=0.99, tol=0.01).do_iterations()
