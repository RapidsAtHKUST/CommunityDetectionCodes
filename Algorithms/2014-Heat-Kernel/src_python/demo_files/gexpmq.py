"""
demo_gexpmq.py

A demonstration of the Gauss-Seidel coordinate relaxation scheme
to estimate a column of the matrix exponential that operates only on significant entries.
Written by Kyle Kloster and David F. Gleich
"""

import collections

from main_files.util_helper import *

if __name__ == '__main__':
    my_graph = get_sample_graph()
    edge_num = get_edge_num(my_graph)
    eps = 0.000917

    N = 6
    col_num = 1
    psis = compute_psis(N)
    thresholds = compute_thresholds(eps, N, psis)

    x = {}  # Store x, r as dictionaries
    r = {}  # initialize residual
    Q = collections.deque()  # initialize queue
    sum_residual = 0.
    r[(col_num, 0)] = 1.
    Q.append(col_num)
    sum_residual += psis[0]

    # Main loop
    for j in xrange(0, N):
        qsize = len(Q)
        relaxtol = thresholds[j] / float(qsize)
        for qi in xrange(0, qsize):
            i = Q.popleft()  # v has r[(v,j)] ...
            rij = r[(i, j)]
            if rij < relaxtol:
                continue

            # perform the relax step
            if i not in x:
                x[i] = 0.
            x[i] += rij
            r[(i, j)] = 0.
            sum_residual -= rij * psis[j]
            update = (rij / (float(j) + 1.)) / len(my_graph[i])

            for u in my_graph[i]:  # for neighbors of i
                next = (u, j + 1)
                if j == N - 1:
                    if u not in x:
                        x[u] = 0.
                    x[u] += update
                else:
                    if next not in r:
                        r[next] = 0.
                        Q.append(u)
                    r[next] += update
                    sum_residual += update * psis[j + 1]
            # after all neighbors u
            if sum_residual < eps:
                break
        if len(Q) == 0:
            break
        if sum_residual < eps:
            break

    for v in xrange(1, len(my_graph) + 1):
        if v in x:
            print "x[%2i] = %.16lf" % (v, x[v])
        else:
            print "x[%2i] = -0." % (v)
