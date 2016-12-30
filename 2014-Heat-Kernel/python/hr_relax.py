"""
hkrelax.py
A demonstration of a relaxation method for computing a heat-kernel based
community that implements the algorith from "Heat-kernel based community
detection" by Kloster & Gleich.
Written by Kyle Kloster and David F. Gleich
refactored by Yulin CHE
"""

import collections
import math

from util_helper import *

if __name__ == '__main__':
    G = get_sample_graph()
    print G
    Gvol = 102

    ## Setup parameters that can be computed automatically
    N = 11  # see paper for how to set this automatically
    t = 5.
    eps = 0.01
    seed = [1]
    psis = compute_psis(N, t)

    ## Estimate hkpr vector
    # G is graph as dictionary-of-sets,
    # t, tol, N, psis are precomputed
    x = {}  # Store x, r as dictionaries
    r = {}  # initialize residual
    Q = collections.deque()  # initialize queue
    for s in seed:
        r[(s, 0)] = 1. / len(seed)
        Q.append((s, 0))
    while len(Q) > 0:
        (v, j) = Q.popleft()  # v has r[(v,j)] ...
        rvj = r[(v, j)]
        # perform the hk-relax step
        if v not in x: x[v] = 0.
        x[v] += rvj
        r[(v, j)] = 0.
        mass = (t * rvj / (float(j) + 1.)) / len(G[v])
        for u in G[v]:  # for neighbors of v
            next = (u, j + 1)  # in the next block
            if j + 1 == N:
                x[u] += rvj / len(G[v])
                continue
            if next not in r: r[next] = 0.
            thresh = math.exp(t) * eps * len(G[u])
            thresh /= N * psis[j + 1]
            if r[next] < thresh <= r[next] + mass:
                Q.append(next)  # add u to queue
            r[next] += mass

    # Find cluster, first normalize by degree
    for v in x: x[v] /= len(G[v])

    for v in xrange(1, len(G) + 1):
        if v in x:
            print "hk[%2i] = %.16lf" % (v, x[v])
        else:
            print "hk[%2i] = -0." % (v)
    print

    # Step 2 do a sweep cut based on this vector
    # now sort x's keys by value, decreasing
    sv = sorted(x.iteritems(), key=lambda x: x[1], reverse=True)
    S = set()
    volS = 0.
    cutS = 0.
    bestcond = 1.
    bestset = sv[0]
    for p in sv:
        s = p[0]  # get the vertex
        volS += len(G[s])  # add degree to volume
        for v in G[s]:
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
