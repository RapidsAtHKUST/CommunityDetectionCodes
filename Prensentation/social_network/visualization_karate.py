import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout


def traverse_degree(graph):
    print "Node Degree"
    for v in graph:
        print '%s %s' % (v, graph.degree(v))


if __name__ == '__main__':
    graph = nx.karate_club_graph()

    nx.draw_networkx(graph)
    plt.show()

    nx.draw_spectral(graph)
    plt.show()

    nx.draw(graph, pos=graphviz_layout(graph), with_labels=True)
    plt.show()

    nx.draw_circular(graph)
    plt.show()

    nx.draw_spring(graph)
    plt.show()
