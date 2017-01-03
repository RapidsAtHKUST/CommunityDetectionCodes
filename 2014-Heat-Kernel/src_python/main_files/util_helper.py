import math
import bvg
import time


def get_sample_graph():
    with open('../dataset/graph_eval_script.txt') as ifs:
        eval_line = ''.join(map(lambda ele: ele.strip(), ifs.readlines()))
    return eval(eval_line)


def get_edge_num(adj_list_dict):
    edge_num = 0
    for vertex in adj_list_dict:
        edge_num += len(adj_list_dict[vertex])
    return edge_num


def compute_psis(N, t=1):
    psis = {N: 1.}
    for i in xrange(N - 1, 0, -1):
        psis[i] = psis[i + 1] * t / (float(i + 1.)) + 1.
    return psis


def compute_thresholds(eps, N, psis):
    threshold_dict = {0: (math.exp(1) * eps / float(N)) / psis[0]}
    for j in xrange(1, N + 1):
        threshold_dict[j] = threshold_dict[j - 1] * psis[j - 1] / psis[j]
    return threshold_dict


def load_twitter_graph():
    print "Loading graph ..."
    start_time = time.time()
    twitter_graph = bvg.BVGraph('../dataset/twitter-2010-symm', 1)
    print "   ... done!   %.1f seconds" % (time.time() - start_time)
    time.sleep(0.75)
    print "\n"
    print "Twitter graph has"
    print "nodes:   %i" % (twitter_graph.nverts)
    print "edges:   %i" % (twitter_graph.nedges)
    time.sleep(0.75)
    print "\nt = 15"
    print "eps = 10^-4\n"
    time.sleep(0.75)
    return twitter_graph
