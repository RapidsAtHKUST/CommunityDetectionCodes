import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout


def get_comm_dict_and_partition():
    comm_dict = {0: [0, 1, 2, 3, 7, 8, 12, 13, 14, 15, 17, 18, 19, 20, 21, 22, 23, 26, 27, 28, 29, 30, 31, 32, 33],
                 1: [24, 25, 31],
                 2: [0, 16, 4, 5, 6, 10]}
    partition_dict = {}
    for comm_id in comm_dict:
        for node in comm_dict[comm_id]:
            partition_dict[node] = comm_id
    print partition_dict
    return comm_dict, partition_dict


def draw_comm_detection_res(graph):
    pos = graphviz_layout(graph)
    color_list = ['r', 'g', 'b', 'y']
    comm_dict, partition = get_comm_dict_and_partition()

    # nodes
    for comm_id in comm_dict:
        nx.draw_networkx_nodes(graph, pos, nodelist=comm_dict[comm_id],
                               node_color=color_list[comm_id], node_size=500, alpha=0.8)

    nx.draw_networkx_nodes(graph, pos, nodelist=[9, 11],
                           node_color='w', node_size=500, alpha=0.8)

    nx.draw_networkx_nodes(graph, pos, nodelist=[31], node_color='y', node_size=500, alpha=0.8)

    nx.draw_networkx_nodes(graph, pos, nodelist=[0], node_color='magenta', node_size=500, alpha=0.8)

    nx.draw_networkx_edges(graph, pos, width=4.0, alpha=0.5, edge_color='grey')

    # labels
    nx.draw_networkx_labels(graph, pos, font_size=16)
    plt.axis('off')
    plt.savefig('./clique_percolation_karate_partition.pdf', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.savefig('./clique_percolation_karate_partition.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


if __name__ == '__main__':
    graph = nx.karate_club_graph()
    draw_comm_detection_res(graph)
