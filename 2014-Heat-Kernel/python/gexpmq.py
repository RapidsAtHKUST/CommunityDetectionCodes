"""
demo_gexpmq.py

A demonstration of the Gauss-Seidel coordinate relaxation scheme
to estimate a column of the matrix exponential that operates
only on significant entries.

Written by Kyle Kloster and David F. Gleich
"""

import collections

from util_helper import *

if __name__ == '__main__':

    # setup the graph
    G = get_sample_graph()
    Gvol = 102
    eps = 0.000917

    # Estimate column c of
    # the matrix exponential vector
    # G is the graph as a dictionary-of-sets,
    # eps is set to stopping tolerance

    # Setup parameters and constants
    N = 6
    c = 1  # the column to compute
    psis = compute_psis(N)
    threshs = compute_threshs(eps, N, psis)
    # Initialize variables
    x = {}  # Store x, r as dictionaries
    r = {}  # initialize residual
    Q = collections.deque()  # initialize queue
    sumresid = 0.
    r[(c, 0)] = 1.
    Q.append(c)
    sumresid += psis[0]
    # Main loop
    for j in xrange(0, N):
        qsize = len(Q)
        relaxtol = threshs[j] / float(qsize)
        for qi in xrange(0, qsize):
            i = Q.popleft()  # v has r[(v,j)] ...
            rij = r[(i, j)]
            if rij < relaxtol:
                continue
            # perform the relax step
            if i not in x: x[i] = 0.
            x[i] += rij
            r[(i, j)] = 0.
            sumresid -= rij * psis[j]
            update = (rij / (float(j) + 1.)) / len(G[i])
            for u in G[i]:  # for neighbors of i
                next = (u, j + 1)
                if j == N - 1:
                    if u not in x: x[u] = 0.
                    x[u] += update
                else:
                    if next not in r:
                        r[next] = 0.
                        Q.append(u)
                    r[next] += update
                    sumresid += update * psis[j + 1]
            # after all neighbors u
            if sumresid < eps: break
        if len(Q) == 0: break
        if sumresid < eps: break

    for v in xrange(1, len(G) + 1):
        if v in x:
            print "x[%2i] = %.16lf" % (v, x[v])
        else:
            print "x[%2i] = -0." % (v)
