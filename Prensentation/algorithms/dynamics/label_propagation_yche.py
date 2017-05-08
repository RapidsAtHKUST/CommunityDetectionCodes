import networkx as nx
import random

if __name__ == '__main__':
    g = nx.read_edgelist('example_edge_list.txt', nodetype=int)


    def get_rand_element(l):
        if l:
            return l[random.randint(0, len(l) - 1)]
        else:
            return None


    def reverse_dict(original_dict):
        reversed_dict = {}
        for key, val in original_dict.iteritems():
            if val not in reversed_dict:
                reversed_dict[val] = []
            reversed_dict[val].append(key)
        return reversed_dict


    def print_result(graph, label_name):
        print ' '.join(map(lambda node: str(node) + ':' + str(graph.node[node][label_name]), graph.nodes()))


    def sync_label_propagation():
        iter_num = 10
        for node in nx.nodes(g):
            g.node[node]['prev'] = node
            g.node[node]['next'] = None

        for i in xrange(iter_num):
            for node in nx.nodes(g):
                label_count_dict = {}

                for neighbor in nx.neighbors(g, node):
                    if g.node[neighbor]['prev'] not in label_count_dict:
                        label_count_dict[g.node[neighbor]['prev']] = 0
                    label_count_dict[g.node[neighbor]['prev']] += 1

                label_list_dict = reverse_dict(label_count_dict)
                tie_list = label_list_dict[max(list(label_list_dict))]
                print label_count_dict
                print tie_list
                g.node[node]['next'] = get_rand_element(tie_list)

            for node in nx.nodes(g):
                g.node[node]['prev'] = g.node[node]['next']

            print_result(g, 'prev')


    def async_label_propagation():
        iter_num = 1
        for node in nx.nodes(g):
            g.node[node]['label'] = node
        for i in xrange(iter_num):
            for node in nx.nodes(g):
                label_count_dict = {}

                for neighbor in nx.neighbors(g, node):
                    if g.node[neighbor]['label'] not in label_count_dict:
                        label_count_dict[g.node[neighbor]['label']] = 0
                    label_count_dict[g.node[neighbor]['label']] += 1

                label_list_dict = reverse_dict(label_count_dict)
                tie_list = label_list_dict[max(list(label_list_dict))]
                print label_count_dict
                print tie_list
                g.node[node]['label'] = get_rand_element(tie_list)

            print_result(g, 'label')


    # sync_label_propagation()
    async_label_propagation()
