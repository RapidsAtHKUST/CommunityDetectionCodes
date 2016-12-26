__author__ = "Giulio Rossetti"
__contact__ = "giulio.rossetti@isti.cnr.it"
__license__ = "MIT"

import networkx as nx
import random


class HDemon(object):
    """
    Hierarchical version of Demon algorithm as described in

    M. Coscia, G. Rossetti, F. Giannotti, D. Pedreschi:
    Uncovering Hierarchical and Overlapping Communities with a Local-First Approach, TKDD 2015
    """

    def __init__(self, g, min_community_size=0, epsilon=0.25):
        """
        Constructor

        :@param g: the networkx graph on which perform Demon
        :@param min_community_size: min nodes needed to form a community
        :@param epsilon: min jacard among a community pair in order to generate an edge in the new graph
        """
        self.actual_com_id = 0
        self.g = g
        self.min_community_size = min_community_size
        self.epsilon = epsilon

    def execute(self):
        """
        Execute HDemon algorithm

        As results 2 groups of files are produced:
        graph-*: edge list of the graph for the level *
        communities-*: communities  for the level *. For level > 0 the nodes have to be intended
                        as "meta nodes" representing communities of the previous level

        """

        total_nodes = len(nx.nodes(self.g))
        actual = 0

        level = 0

        while total_nodes > nx.number_connected_components(self.g) or level == 0:
            print "\n------------------------\n" \
                  "Starting level %d: nodes to process %d\n" % (level, total_nodes)

            out_file_graph = open("graph-%d" % level, "w")
            out_file_comm = open("communities-%d" % level, "w")

            for n in self.g.nodes():
                self.g.node[n]['communities'] = [n]

            actual_coms = {}

            for ego in nx.nodes(self.g):

                print "Ego-network analyzed: %d/%d" % (actual, total_nodes)
                actual += 1

                ego_minus_ego = nx.ego_graph(self.g, ego, 1, False)

                self.__overlapping_label_propagation(ego_minus_ego, ego, actual_coms,
                                                     out_file_graph, out_file_comm)
            actual = 0

            out_file_graph.close()
            out_file_comm.close()

            # build the new graph based on the communities detected at the end of this LP step
            self.g = None
            self.g = nx.Graph()
            edges = open("graph-%d" % level, "r")
            for e in edges:
                part = e.split('\t')
                self.g.add_edge(int(part[0]), int(part[1]))

            total_nodes = len(self.g.nodes())
            level += 1

        print "\nComputation ended: results available in the files graph-* and communities-*\n"
        return


    def __build_graph_from_communities(self, communities):
        """

        :@param communities: set of communities to analyze
        """
        go = nx.Graph()

        for c1 in communities:
            for c2 in communities:
                if set(c1) == set(c2):
                    pass
                else:
                    w = float(len(set(c1) & set(c2)))/len(set(c1) | set(c2))
                    if w > self.epsilon:
                        go.add_edge(int(communities[c1]), int(communities[c2]))

        return go

    def __overlapping_label_propagation(self, ego_minus_ego, ego, actual_coms, out_file_graph, out_file_com,
                                        max_iteration=10):
        """
        :@param actual_coms:
        :@param out_file_graph:
        :@param out_file_com:
        :@param max_iteration: number of desired iteration for the label propagation
        :@param ego_minus_ego: ego network minus its center
        :@param ego: ego network center
        """

        t = 0

        old_node_to_coms = {}

        while t < max_iteration:
            t += 1

            node_to_coms = {}

            nodes = nx.nodes(ego_minus_ego)
            random.shuffle(nodes)

            count = -len(nodes)

            for n in nodes:

                label_freq = {}

                n_neighbors = nx.neighbors(ego_minus_ego, n)
                if len(n_neighbors) < 1:
                    continue

                if count == 0:
                    t += 1

                # compute the frequency of the labels
                for nn in n_neighbors:

                    communities_nn = [nn]

                    if nn in old_node_to_coms:
                        communities_nn = old_node_to_coms[nn]

                    for nn_c in communities_nn:
                        if nn_c in label_freq:
                            v = label_freq.get(nn_c)
                            label_freq[nn_c] = v + 1
                        else:
                            label_freq[nn_c] = 1

                # first run, random choosing of the communities among the neighbors labels
                if t == 1:
                    if not len(n_neighbors) == 0:
                        r_label = random.sample(label_freq.keys(), 1)
                        ego_minus_ego.node[n]['communities'] = r_label
                        old_node_to_coms[n] = r_label
                    count += 1
                    continue

                # choose the majority
                else:
                    labels = []
                    max_freq = -1

                    for l, c in label_freq.items():
                        if c > max_freq:
                            max_freq = c
                            labels = [l]
                        elif c == max_freq:
                            labels.append(l)

                    node_to_coms[n] = labels

                    if n not in old_node_to_coms or not set(node_to_coms[n]) == set(old_node_to_coms[n]):
                        old_node_to_coms[n] = node_to_coms[n]
                        ego_minus_ego.node[n]['communities'] = labels

            t += 1

        # build the communities reintroducing the ego
        community_to_nodes = {}
        com_to_id = {}
        for n in nx.nodes(ego_minus_ego):

            if len(nx.neighbors(ego_minus_ego, n)) == 0:
                ego_minus_ego.node[n]['communities'] = [n]

            c_n = ego_minus_ego.node[n]['communities']
            for c in c_n:
                if c in community_to_nodes:
                    com = community_to_nodes.get(c)
                    com.append(n)
                else:
                    com_to_id[c] = self.actual_com_id
                    self.actual_com_id += 1
                    nodes = [n, ego]
                    community_to_nodes[c] = nodes

        nodes_to_com_id = {}
        for c in community_to_nodes:

            if len(community_to_nodes[c]) > self.min_community_size and not tuple(
                    sorted(community_to_nodes[c])) in actual_coms:
                # write the community
                actual_coms[tuple(sorted(community_to_nodes[c]))] = com_to_id[c]
                out_file_com.write("%d\t%s\n" % (com_to_id[c], str(sorted(community_to_nodes[c]))))

                # write the edges for the new graph
        for c2 in actual_coms:
            for c in actual_coms:
                if c > c2 and (len(c2) > self.min_community_size) and (len(c) > self.min_community_size):
                    out_file_graph.write("%d\t%d\n" % (actual_coms[c], actual_coms[c2]))

        return len(nodes_to_com_id)


###############################
g = nx.Graph()
fin = open("network.csv")
for l in fin:
    l = l.rstrip().split(",")
    g.add_edge(l[0], l[1])

d = HDemon(g, epsilon=0.25)
d.execute()
