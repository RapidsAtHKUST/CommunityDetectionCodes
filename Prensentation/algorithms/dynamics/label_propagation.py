import random
import networkx as nx


def propagate_label_sync(graph, max_iteration=100):
    old_label_dict = {}

    def rand_init_labels():
        for n in graph.nodes():
            # r_label = random.sample(graph.nodes(), 1) if len(graph.neighbors(n)) != 0 else [n]
            r_label = [n]
            graph.node[n]['communities'] = r_label
            old_label_dict[n] = r_label
        return

    rand_init_labels()

    for iter_num in xrange(max_iteration):
        node_to_coms = {}

        nodes = list(nx.nodes(graph))
        random.shuffle(nodes)

        for n in graph.nodes():
            n_neighbors = nx.neighbors(graph, n)

            if len(n_neighbors) < 1:
                continue

            label_freq_dict = {}

            def update_label_freq_dict():
                for nn in n_neighbors:
                    communities_nn = [nn]
                    if nn in old_label_dict:
                        communities_nn = old_label_dict[nn]

                    for nn_c in communities_nn:
                        if nn_c not in label_freq_dict:
                            label_freq_dict[nn_c] = 1
                        label_freq_dict[nn_c] += 1

            update_label_freq_dict()

            labels = []
            max_freq = -1
            for l, c in label_freq_dict.items():
                if c > max_freq:
                    max_freq = c
                    labels = [l]
                elif c == max_freq:
                    labels.append(l)

            node_to_coms[n] = random.sample(labels, 1)

            if n not in old_label_dict or set(node_to_coms[n]) != set(old_label_dict[n]):
                old_label_dict[n] = node_to_coms[n]
                graph.node[n]['communities'] = labels

        print old_label_dict


if __name__ == '__main__':
    karate_graph = nx.karate_club_graph()
    propagate_label_sync(karate_graph)
    print
