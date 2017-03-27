import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout
from disjoint_community_detection import *


def traverse_degree(graph):
    print "Node Degree"
    for v in graph:
        print '%s %s' % (v, graph.degree(v))


def try_different_layouts(graph):
    nx.draw_random(graph, with_labels=True)
    plt.show()

    nx.draw_circular(graph, with_labels=True)
    plt.show()

    nx.draw_spectral(graph, with_labels=True)
    plt.show()

    nx.draw_networkx(graph, with_labels=True)
    plt.show()

    nx.draw(graph, pos=graphviz_layout(graph), with_labels=True)
    plt.show()

    nx.draw_shell(graph, with_labels=True)
    plt.show()

    nx.draw_spring(graph, with_labels=True)
    plt.show()


if __name__ == '__main__':
    graph = nx.karate_club_graph()
    pos = graphviz_layout(graph)
    color_list = ['r', 'g', 'b', 'y']
    comm_dict, partition = get_comm_dict_and_partition(graph)

    # nodes
    for comm_id in comm_dict:
        nx.draw_networkx_nodes(graph, pos, nodelist=comm_dict[comm_id],
                               node_color=color_list[comm_id], node_size=500, alpha=0.8)


    # edges
    def get_edge_dict():
        edge_dict = {}
        for comm_id in comm_dict:
            edge_dict[comm_id] = []

        for edge in graph.edges():
            if partition[edge[0]] == partition[edge[1]]:
                edge_dict[partition[edge[0]]].append(edge)
        return edge_dict


    edge_list_dict = get_edge_dict()
    nx.draw_networkx_edges(graph, pos, width=4.0, alpha=0.5, edge_color='grey')
    for comm_id in edge_list_dict:
        print comm_id, color_list[comm_id], edge_list_dict[comm_id]
        nx.draw_networkx_edges(graph, pos, edgelist=edge_list_dict[comm_id],
                               width=8, alpha=0.5, edge_color=color_list[comm_id])

    # labels
    nx.draw_networkx_labels(graph, pos, font_size=16)
    plt.axis('off')
    plt.savefig('./karate_partition.pdf', bbox_inches='tight', pad_inches=0)
    plt.show()
