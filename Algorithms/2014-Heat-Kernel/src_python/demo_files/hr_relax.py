"""
A demonstration of a relaxation method for computing a heat-kernel based community
that implements the algorithm from "Heat-kernel based community detection" by Kloster & Gleich.
Written by Kyle Kloster and David F. Gleich
refactored by Yulin CHE
"""

import collections

from main_files.util_helper import *

if __name__ == '__main__':
    graph = get_sample_graph()
    Gvol = 102

    # Setup parameters that can be computed automatically
    N = 11
    t = 5.
    eps = 0.01
    seed_list = [1]
    psis = compute_psis(N, t)

    # Estimate hkpr vector
    x_dict = {}
    residual_dict = {}
    task_queue = collections.deque()

    for s in seed_list:
        residual_dict[(s, 0)] = 1. / len(seed_list)
        task_queue.append((s, 0))

    while len(task_queue) > 0:
        (v, j) = task_queue.popleft()  # v has r[(v,j)] ...
        rvj = residual_dict[(v, j)]

        # perform the hk-relax step
        if v not in x_dict:
            x_dict[v] = 0.
        x_dict[v] += rvj
        residual_dict[(v, j)] = 0.
        mass = (t * rvj / (float(j) + 1.)) / len(graph[v])

        # for neighbors of v
        for u in graph[v]:
            next_block = (u, j + 1)
            if j + 1 == N:
                x_dict[u] += rvj / len(graph[v])
                continue

            if next_block not in residual_dict:
                residual_dict[next_block] = 0.
            thresh = math.exp(t) * eps * len(graph[u])
            thresh /= N * psis[j + 1]
            if residual_dict[next_block] < thresh <= residual_dict[next_block] + mass:
                task_queue.append(next_block)  # add u to queue
            residual_dict[next_block] += mass

    # Find cluster, first normalize by degree
    for v in x_dict:
        x_dict[v] /= len(graph[v])

    for v in xrange(1, len(graph) + 1):
        if v in x_dict:
            print "hk[%2i] = %.16lf" % (v, x_dict[v])
        else:
            print "hk[%2i] = -0." % (v)

    # Step 2 do a sweep cut based on this vector
    # now sort x's keys by value, decreasing
    sv = sorted(x_dict.iteritems(), key=lambda x: x[1], reverse=True)
    S = set()
    volS = 0.
    cutS = 0.
    bestcond = 1.
    bestset = sv[0]
    for p in sv:
        s = p[0]  # get the vertex
        volS += len(graph[s])  # add degree to volume
        for v in graph[s]:
            if v in S:
                cutS -= 1
            else:
                cutS += 1
        print "v: %4i  cut: %4f  vol: %4f" % (s, cutS, volS)
        S.add(s)
        if cutS / min(volS, Gvol - volS) < bestcond:
            bestcond = cutS / min(volS, Gvol - volS)
            bestset = set(S)  # make a copy
    print "Best set conductance: %f" % (bestcond)
    print "  set = ", str(bestset)
