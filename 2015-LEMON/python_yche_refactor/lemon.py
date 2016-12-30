#!/usr/bin/env python
# encoding:UTF-8

import math
import pulp

from copy import deepcopy
from scipy import linalg as splin
from optparse import OptionParser

from io_helper import *


def set_initial_prob(n, starting_nodes):
    v = np.zeros(n)
    v[starting_nodes] = 1. / starting_nodes.size
    return v


def set_initial_prob_proportional(n, degree_sequence, starting_nodes):
    v = np.zeros(n)
    vol = 0
    for node in starting_nodes:
        vol += degree_sequence[node]
    for node in starting_nodes:
        v[node] = degree_sequence[node] / float(vol)
    return v


def adj_to_laplacian(G):
    n = G.shape[0]
    D = np.zeros((1, n))
    for i in range(n):
        D[0, i] = math.sqrt(G[i, :].sum())

    temp = np.dot(D.T, np.ones((1, n)))
    horizontal = G / temp
    normalized_adjacency_matrix = horizontal / (temp.T)
    gc.collect()

    return normalized_adjacency_matrix


def cal_conductance(G, cluster):
    assert type(cluster) == np.ndarray, "The given community members is not a numpy array"

    temp = G[cluster, :]
    subgraph = temp[:, cluster]
    cutsize = temp.sum() - subgraph.sum()
    denominator = min(temp.sum(), G.sum() - temp.sum())
    conductance = cutsize / denominator

    return conductance


def sample_graph(G_linklist, node_number, degree_sequence, starting_node, sample_rate=None, biased=True):
    initial = np.array(starting_node)
    if biased:
        prob_distribution = set_initial_prob_proportional(node_number, degree_sequence, initial)
    else:
        prob_distribution = set_initial_prob(node_number, initial)

    subgraph = set(starting_node)
    RW_graph = set(starting_node)

    for j in range(30):
        original_distribution = deepcopy(prob_distribution)
        for node in RW_graph:
            neighbors = G_linklist[node]
            divided_prob = original_distribution[node] / float(len(neighbors))
            prob_distribution[node] -= original_distribution[node]
            for v in neighbors:
                prob_distribution[v] += divided_prob
            RW_graph = RW_graph.union(neighbors)

            if len(RW_graph) >= 7000:
                break

    for i in range(30):
        for node in subgraph:

            neighbors = G_linklist[node]
            subgraph = subgraph.union(neighbors)
            if len(subgraph) >= 7000:
                break

    index = 0
    RW_dict = {}
    RW_dict_reverse = {}
    sub_prob_distribution = []
    for node in subgraph:
        RW_dict[node] = index  # mapping from whole graph to subgraph
        RW_dict_reverse[index] = node  # mapping from subgraph to whole graph
        sub_prob_distribution.append(prob_distribution[node])
        index += 1

    new_graph_size = 3000

    node_in_new_graph = list(np.argsort(sub_prob_distribution)[::-1][:new_graph_size])
    node_in_new_graph_ori_index = []
    for v in node_in_new_graph:
        node_in_new_graph_ori_index.append(RW_dict_reverse[v])
    node_in_new_graph_ori_index = set(node_in_new_graph_ori_index)

    print "The size of sampled graph is", len(node_in_new_graph)
    new_graph = np.eye(new_graph_size)
    map_dict = {}
    map_dict_reverse = {}
    count = 0
    for i in range(node_number):
        if i in node_in_new_graph_ori_index:
            map_dict[count] = i  # from new to original
            map_dict_reverse[i] = count  # from original to new
            count += 1

    for i in range(new_graph_size):
        for j in range(new_graph_size):
            a = map_dict[i]
            b = map_dict[j]
            if a in G_linklist[b]:
                new_graph[i, j] = 1.0
                new_graph[j, i] = 1.0
    print "new graph returned"
    sample_rate = new_graph_size / float(node_number)

    return new_graph, map_dict, map_dict_reverse, sample_rate, new_graph_size


def map_from_ori_to_new(nodelist, map_dict_reverse):
    return np.array(
        [map_dict_reverse[node] if node in map_dict_reverse else 10000000000 for node in np.array(nodelist)])


def random_walk(G, initial_prob, subspace_dim=3, walk_steps=3):
    assert type(initial_prob) == np.ndarray, "Initial probability distribution is not a numpy array"

    # Transform the adjacent matrix to a laplacian matrix P
    P = adj_to_laplacian(G)

    prob_matrix = np.zeros((G.shape[0], subspace_dim))
    prob_matrix[:, 0] = initial_prob
    for i in range(1, subspace_dim):
        prob_matrix[:, i] = np.dot(prob_matrix[:, i - 1], P)

    orth_prob_matrix = splin.orth(prob_matrix)

    for i in range(walk_steps):
        temp = np.dot(orth_prob_matrix.T, P)
        orth_prob_matrix = splin.orth(temp.T)
    return orth_prob_matrix


def min_one_norm(B, initial_seed, seed):
    weight_initial = 1 / float(len(initial_seed))
    weight_later_added = weight_initial / float(0.5)
    difference = len(seed) - len(initial_seed)
    [r, c] = B.shape
    prob = pulp.LpProblem("Minimum one norm", pulp.LpMinimize)
    indices_y = range(0, r)
    y = pulp.LpVariable.dicts("y_s", indices_y, 0)
    indices_x = range(0, c)
    x = pulp.LpVariable.dicts("x_s", indices_x)

    f = dict(zip(indices_y, [1.0] * r))

    prob += pulp.lpSum(f[i] * y[i] for i in indices_y)  # objective function
    prob += pulp.lpSum(y[s] for s in initial_seed) >= 1
    prob += pulp.lpSum(y[r] for r in seed) >= 1 + weight_later_added * difference

    for j in range(r):
        temp = dict(zip(indices_x, list(B[j, :])))
        prob += pulp.lpSum(y[j] + (temp[k] * x[k] for k in indices_x)) == 0

    prob.solve()

    print "Status:", pulp.LpStatus[prob.status]
    result = []
    for var in indices_y:
        result.append(y[var].value())
    return result


def seed_expand_auto(G, seedset, min_comm_size, max_comm_size, expand_step=None, subspace_dim=None, walk_steps=None,
                     biased=True):
    degree = []
    n = G.shape[0]
    for x in range(n):
        degree.append(G[x].sum())

    # Random walk starting from seed nodes:
    if biased:
        initial_prob = set_initial_prob_proportional(n, degree, seedset)
    else:
        initial_prob = set_initial_prob(G.shape[0], seedset)

    Orth_Prob_Matrix = random_walk(G, initial_prob, subspace_dim, walk_steps)
    initial_seed = seedset

    # Initialization
    detected = list(seedset)
    seed = seedset
    step = expand_step
    detected_comm = []

    global_conductance = np.zeros(30)
    global_conductance[-1] = 1000000  # set the last element to be infinitely large
    global_conductance[-2] = 1000000
    flag = True

    iteration = 0
    while flag:
        temp = np.argsort(np.array(min_one_norm(Orth_Prob_Matrix, list(initial_seed), list(seed))))

        sorted_top = list(temp[::-1][:step])

        detected = list(set(list(detected) + sorted_top))
        seed = np.array(detected)

        conductance_record = np.zeros(max_comm_size - min_comm_size + 1)
        conductance_record[-1] = 0
        for i in range(min_comm_size, max_comm_size):
            candidate_comm = np.array(list(temp[::-1][:i]))
            conductance_record[i - min_comm_size] = cal_conductance(G, candidate_comm)

        detected_size, cond = global_minimum(conductance_record, min_comm_size)

        step += expand_step

        if biased:
            initial_prob = set_initial_prob_proportional(n, degree, seedset)
        else:
            initial_prob = set_initial_prob(G.shape[0], seedset)

        Orth_Prob_Matrix = random_walk(G, initial_prob, subspace_dim, walk_steps)

        if detected_size != 0:
            current_comm = list(temp[::-1][:detected_size])
            detected_comm = current_comm

        global_conductance[iteration] = cond
        if global_conductance[iteration - 1] <= global_conductance[iteration] and \
                        global_conductance[iteration - 1] <= global_conductance[iteration - 2]:
            flag = False

        iteration += 1
    return detected_comm


def global_minimum(sequence, start_index):
    detected_size = len(list(sequence))
    seq_length = len(list(sequence))
    cond = sequence[seq_length - 2]
    for x in range(40):
        list(sequence).append(0)
    for i in range(seq_length - 40):
        if sequence[i] < sequence[i - 1] and sequence[i] < sequence[i + 1]:
            count_larger = 0
            count_smaller = 0
            for j in range(1, 32):
                if sequence[i + 1 + j] > sequence[i + 1]:
                    count_larger += 1
            for k in range(1, 32):
                if sequence[i - 1 - k] > sequence[i - 1]:
                    count_smaller += 1
            if count_larger >= 18 and count_smaller >= 18:
                print "first glocal minimum found:", i + start_index
                detected_size = i + start_index
                cond = sequence[i]
                break
    return detected_size, cond


def cal_f_score(detected_comm, ground_truth_comm, beta=1):
    detected_comm = list(detected_comm)
    ground_truth_comm = list(ground_truth_comm)
    correctly_classified = list(set(detected_comm).intersection(set(ground_truth_comm)))
    precision = len(correctly_classified) / float(len((detected_comm)))
    recall = len(correctly_classified) / float(len(ground_truth_comm))
    if precision != 0 and recall != 0:
        Fscore = (1 + math.sqrt(beta)) / float(math.sqrt(beta)) * precision * recall / float(precision + recall)
    else:
        Fscore = 0
    return Fscore


class MyParser(OptionParser):
    def format_epilog(self, formatter):
        return self.epilog


if __name__ == '__main__':
    usage = "usage: python lemon.py [options]"
    parser = MyParser(usage)
    parser.add_option("-d", "--delimiter", dest="delimiter", default=' ',
                      help="delimiter of input & output files [default: space]")
    parser.add_option("-f", "--input network file", dest="network_file", default="../example/amazon/graph",
                      help="input file of edge list for clustering [default: example_graphs/amazon/graph]")
    parser.add_option("-g", "--input community ground truth file", dest="groundtruth_community_file",
                      default="../example/amazon/community",
                      help="input file of ground truth community membership [default: example_graphs/amazon/community]")
    parser.add_option("--out", "--output file", dest="output_file", default="output.txt",
                      help="output file of detected community [default: output.txt]")
    parser.add_option("--sd", "--input seed set file", dest="seed_set_file", default="../example/amazon/seed",
                      help="input file of initial seed set [default: example_graphs/amazon/seed]")
    parser.add_option("-c", "--minimum community size", dest="min_comm_size", default=20,
                      help="the minimum size of a single community in the network [default: 50]")
    parser.add_option("-C", "--maximal community size", dest="max_comm_size", default=100,
                      help="the maximum size of a single community in the network [default: 400]")
    parser.add_option("-s", "--sample rate", dest="sample_rate", default=0.007,
                      help="the percentile of nodes left for local detection after sampling [default: 0.007]")
    parser.add_option("-e", "--expand step", dest="expand_step", default=6,
                      help="the step of seed set increasement during expansion process [default: 6]")
    parser.add_option("-z", "--seed set size", dest="ini_num_of_seed", default=3,
                      help="the initial number of seeds used for expansion [default: 3]")
    (options, args) = parser.parse_args()
    delimiter = options.delimiter
    network_file = options.network_file
    community_file = options.groundtruth_community_file
    output_file = options.output_file
    seed_set_file = options.seed_set_file
    min_comm_size = int(options.min_comm_size)
    max_comm_size = int(options.max_comm_size)
    sample_rate = float(options.sample_rate)
    expand_step = int(options.expand_step)
    seed_set_size = int(options.ini_num_of_seed)

    comms_indices_map, count = read_ground_truth(community_file, delimiter=delimiter, nodetype=int)
    graph_link_list, node_number, edge_number, degree = read_edge_list(network_file, delimiter=delimiter, nodetype=int)
    with open(seed_set_file) as fin:
        seedset = np.array(map(int, filter(lambda ele: '#' not in ele, fin.readlines())[0].strip().split()))
    seedset -= 1

    with open('eval_ground_truth.txt') as ifs:
        test_comm = np.array(eval(''.join(map(lambda ele: ele.strip(), ifs.readlines()))))
    test_comm -= 1

    # sample the graph, adjust indices
    new_graph, map_dict, map_dict_reverse, sample_rate, new_graph_size \
        = sample_graph(graph_link_list, node_number, degree, seedset, 0.007, biased=False)
    new_seedset = map_from_ori_to_new(seedset, map_dict_reverse)
    new_test_comm = map_from_ori_to_new(test_comm, map_dict_reverse)

    detected_comm = seed_expand_auto(new_graph, new_seedset, min_comm_size, max_comm_size, expand_step, subspace_dim=3,
                                     walk_steps=3, biased=False)
    detected_comm_ori = np.array([map_dict[node] for node in np.array(detected_comm)])

    print "-------------------------------------------------------"
    print "The detected community is:\n", list(detected_comm_ori - 1)
    F1_score = cal_f_score(detected_comm, new_test_comm)
    print "-----------------------------------------------------------------------"
    print "The F1 score between detected community and ground truth community is: ", F1_score

    with open(output_file, "a") as out:
        out.write("# detected community:" + "\n")
        out.write(str(list(detected_comm_ori + 1)))
        out.write('\n')
        out.write('\n')
        out.write("# F1 score: " + str(F1_score) + '\n')
