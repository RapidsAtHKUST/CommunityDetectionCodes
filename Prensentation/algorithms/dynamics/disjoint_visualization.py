import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout
import matplotlib.pyplot as plt


def draw_label_propagation_graph(graph, comm_dict, number):
    pos = graphviz_layout(graph)
    nx.draw(graph, pos, width=4.0, alpha=0.5, edge_color='grey',
            node_color='white', node_size=500, with_labels=True)

    color_dict = {6: 'red', 8: 'green', 10: 'blue', 13: 'yellow', 27: 'pink', 29: 'purple', 31: 'black', 32: 'orange',
                  33: 'cyan'}

    for idx, comm in comm_dict.iteritems():
        nx.draw_networkx_nodes(graph, pos, nodelist=comm, node_color=color_dict[idx], node_size=500, alpha=0.5)

    plt.axis('off')
    plt.savefig('./label_propagation_iter' + str(number) + '.pdf', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.savefig('./label_propagation_iter' + str(number) + '.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


def draw_graph(graph):
    pos = graphviz_layout(graph)
    nx.draw(graph, pos, width=4.0, alpha=0.5, edge_color='grey',
            node_color='white', node_size=500, with_labels=True)
    plt.axis('off')
    plt.savefig('./origin_graph_before_lp' '.pdf', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.savefig('./origin_graph_before_lp' '.png', bbox_inches='tight', pad_inches=0, transparent=True)


if __name__ == '__main__':
    graph = nx.karate_club_graph()
    draw_graph(graph)
    with open('disjoint_community.txt') as ifs:
        comm_id_dict_list = map(lambda line: eval(line.strip()), ifs.readlines())
        for comm_id_dict in comm_id_dict_list:
            for key in comm_id_dict:
                comm_id_dict[key] = comm_id_dict[key][0]

        for comm_id_dict in comm_id_dict_list:
            print comm_id_dict


        def reverse_dict(original_dict):
            new_dict = {}
            for key, val in original_dict.iteritems():
                if val not in new_dict:
                    new_dict[val] = []
                new_dict[val].append(key)
            return new_dict


        new_comm_dict_list = map(reverse_dict, comm_id_dict_list)

    for idx, comm_dict in enumerate(new_comm_dict_list):
        draw_label_propagation_graph(graph, comm_dict, idx)
        print comm_dict
