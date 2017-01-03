import collections

from util_helper import *

if __name__ == '__main__':
    adj_list_dict = get_sample_graph()
    edge_num = get_edge_num(adj_list_dict)
    alpha = 0.99
    tol = 0.01
    seed = [1]

    x_dict = {}
    residual_dict = {}
    task_queue = collections.deque()

    for vertex in seed:
        residual_dict[vertex] = 1 / len(seed)
        task_queue.append(vertex)

    # Personalized PageRank
    while len(task_queue) > 0:
        v = task_queue.popleft()  # v has r[v] > tol*deg(v)
        if v not in x_dict:
            x_dict[v] = 0.
        x_dict[v] += (1 - alpha) * residual_dict[v]
        mass = alpha * residual_dict[v] / (2 * len(adj_list_dict[v]))
        for u in adj_list_dict[v]:  # for neighbors of u
            assert u is not v, "contact dgleich@purdue.edu for self-links"
            if u not in residual_dict:
                residual_dict[u] = 0.
            if residual_dict[u] < len(adj_list_dict[u]) * tol <= residual_dict[u] + mass:
                task_queue.append(u)  # add u to queue if large
            residual_dict[u] += mass
        residual_dict[v] = mass * len(adj_list_dict[v])
        if residual_dict[v] >= len(adj_list_dict[v]) * tol:
            task_queue.append(v)
    print str(x_dict)

    # Find cluster
    for v in x_dict:
        x_dict[v] /= len(adj_list_dict[v])

    sorted_pair = sorted(x_dict.iteritems(), key=lambda x: x[1], reverse=True)

    S = set()
    volS, cutS = 0., 0.
    bestcond = 1.
    bestset = sorted_pair[0]
    for p in sorted_pair:
        vertex = p[0]  # get the vertex
        volS += len(adj_list_dict[vertex])  # add degree to volume
        for v in adj_list_dict[vertex]:
            if v in S:
                cutS -= 1
            else:
                cutS += 1
        print "v: %4i  cut: %4f  vol: %4f" % (vertex, cutS, volS)
        S.add(vertex)
        if cutS / min(volS, edge_num - volS) < bestcond:
            bestcond = cutS / min(volS, edge_num - volS)
            bestset = set(S)  # make a copy
    print "Best set conductance: %f" % (bestcond)
    print "  set = ", str(bestset)
