import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout
import matplotlib.pyplot as plt


def visualize_a_graph(graph):
    pos = graphviz_layout(graph)
    nx.draw(graph, pos, width=4.0, alpha=0.5, edge_color='grey',
            node_color='white', node_size=500, with_labels=True)

    nx.draw_networkx_edge_labels(graph, pos)
    nx.draw_networkx_labels(graph, pos)
    plt.axis('off')
    plt.show()


if __name__ == '__main__':
    G = nx.path_graph(6)
    visualize_a_graph(G)

    partition = [[0, 1], [2, 3], [4, 5]]
    graph = nx.Graph(nx.blockmodel(G, partition, multigraph=True))
    visualize_a_graph(graph)
