import collections

from util_helper import *

if __name__ == '__main__':
    # setup the graph
    G = get_sample_graph()
    Gvol = 102
    alpha = 0.99
    tol = 0.01
    seed = [1]

    x = {}  # Store x, r as dictionaries
    r = {}  # initialize residual
    Q = collections.deque()  # initialize queue
    for s in seed:
        r[s] = 1 / len(seed)
        Q.append(s)
    while len(Q) > 0:
        v = Q.popleft()  # v has r[v] > tol*deg(v)
        if v not in x: x[v] = 0.
        x[v] += (1 - alpha) * r[v]
        mass = alpha * r[v] / (2 * len(G[v]))
        for u in G[v]:  # for neighbors of u
            assert u is not v, "contact dgleich@purdue.edu for self-links"
            if u not in r: r[u] = 0.
            if r[u] < len(G[u]) * tol <= r[u] + mass:
                Q.append(u)  # add u to queue if large
            r[u] += mass
        r[v] = mass * len(G[v])
        if r[v] >= len(G[v]) * tol: Q.append(v)
    print str(x)

    # Find cluster, first normalize by degree
    for v in x:
        x[v] /= len(G[v])

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
