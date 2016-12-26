"""
Created on 18/mar/2013

@author: "Giulio Rossetti"
@contact: giulio.rossetti@isti.cnr.it
"""
import networkx as nx
import random


class Demon(object):
    """
    Michele Coscia, Giulio Rossetti, Fosca Giannotti, Dino Pedreschi:
    DEMON: a local-first discovery method for overlapping communities.
    KDD 2012:615-623
    """

    def __init__(self):
        """
        Constructor
        """

    def execute(self, G, epsilon=0.25, weighted=False, min_community_size=3):
        """
        Execute Demon algorithm

        :param G: the networkx graph on which perform Demon
        :param epsilon: the tolerance required in order to merge communities
        :param weighted: Whether the graph is weighted or not
        :param min_community_size:min nodes needed to form a community
        """

        #######
        self.G = G
        self.epsilon = epsilon
        self.min_community_size = min_community_size
        for n in self.G.nodes():
            G.node[n]['communities'] = [n]
        self.weighted = weighted
        #######

        print  'hello'
        all_communities = {}

        total_nodes = len(list(nx.nodes(self.G)))
        actual = 0
        old_percentage = 0
        for ego in nx.nodes(self.G):
            percentage = float(actual * 100) / total_nodes

            actual += 1

            # ego_minus_ego
            ego_minus_ego = nx.ego_graph(self.G, ego, 1, False)
            community_to_nodes = self.__overlapping_label_propagation(ego_minus_ego, ego)

            # merging phase
            for c in community_to_nodes.keys():
                if len(community_to_nodes[c]) > self.min_community_size:
                    actual_community = community_to_nodes[c]
                    all_communities = self.__merge_communities(all_communities, actual_community)

        # print communities
        out_file_com = open("communities.txt", "w")
        idc = 0
        for c in all_communities.keys():
            out_file_com.write("%d\t%s\n" % (idc, str(sorted(c))))
            idc += 1
        out_file_com.flush()
        out_file_com.close()

        return

    def __overlapping_label_propagation(self, ego_minus_ego, ego, max_iteration=100):
        """

        :param max_iteration: number of desired iteration for the label propagation
        :param ego_minus_ego: ego network minus its center
        :param ego: ego network center
        """
        t = 0

        old_node_to_coms = {}

        while t < max_iteration:
            t += 1

            node_to_coms = {}

            nodes = list(nx.nodes(ego_minus_ego))
            random.shuffle(nodes)

            count = -len(nodes)

            for n in nodes:
                label_freq = {}

                n_neighbors = list(nx.neighbors(ego_minus_ego, n))

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
                            # case of weighted graph
                            if self.weighted:
                                label_freq[nn_c] = v + ego_minus_ego.edge[nn][n]['weight']
                            else:
                                label_freq[nn_c] = v + 1
                        else:
                            # case of weighted graph
                            if self.weighted:
                                label_freq[nn_c] = ego_minus_ego.edge[nn][n]['weight']
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

                    if not n in old_node_to_coms or not set(node_to_coms[n]) == set(old_node_to_coms[n]):
                        old_node_to_coms[n] = node_to_coms[n]
                        ego_minus_ego.node[n]['communities'] = labels

            t += 1

        # build the communities reintroducing the ego
        community_to_nodes = {}
        for n in nx.nodes(ego_minus_ego):
            if len(list(nx.neighbors(ego_minus_ego, n))) == 0:
                ego_minus_ego.node[n]['communities'] = [n]

            c_n = ego_minus_ego.node[n]['communities']

            for c in c_n:

                if c in community_to_nodes:
                    com = community_to_nodes.get(c)
                    com.append(n)
                else:
                    nodes = [n, ego]
                    community_to_nodes[c] = nodes

        return community_to_nodes

    def __merge_communities(self, communities, actual_community):
        """

        :param communities: dictionary of communities
        :param actual_community: a community
        """

        # if the community is already present return
        if tuple(actual_community) in communities:
            return communities

        else:
            # search a community to merge with
            inserted = False

            for test_community in communities.items():

                union = self.__generalized_inclusion(actual_community, test_community[0])
                # communty to merge with found!
                if union is not None:
                    communities.pop(test_community[0])
                    communities[tuple(sorted(union))] = 0
                    inserted = True
                    break

            # not merged: insert the original community
            if not inserted:
                communities[tuple(sorted(actual_community))] = 0

        return communities

    def __generalized_inclusion(self, c1, c2):
        """

        :param c1: community
        :param c2: community
        """
        intersection = set(c2) & set(c1)
        smaller_set = min(len(c1), len(c2))

        if len(intersection) == 0:
            return None

        if not smaller_set == 0:
            res = float(len(intersection)) / float(smaller_set)

        if res >= self.epsilon:
            union = set(c2) | set(c1)
            return union


#################################
g = nx.Graph()
fin = open("/home/cheyulin/GitRepos/SocialNetworkAnalysis/Codes-Yche/demo_input_files/karate_edges_input.csv")
for l in fin:
    l = l.rstrip().split(" ")
    print  l[0] + ' ,' + l[1]
    g.add_edge(l[0], l[1])

d = Demon()
d.execute(g, epsilon=0.25)
