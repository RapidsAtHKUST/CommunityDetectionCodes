import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout


def vis_input(graph):
    # pos = graphviz_layout(graph)
    pos = nx.circular_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=20, node_size=2000, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    plt.axis('off')
    plt.savefig('./demo_graph.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


def vis_post_output(graph, comm, color):
    pos = nx.circular_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=20, node_size=2000, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    nx.draw_networkx_nodes(graph, with_labels=True, pos=pos, font_size=20, node_size=2000, alpha=0.4, width=4,
                           node_color=color, nodelist=comm)

    plt.axis('off')
    plt.savefig('./output_post_graph' + ''.join(map(str, comm)) + '.png', bbox_inches='tight', pad_inches=0,
                transparent=True)
    plt.show()


if __name__ == '__main__':
    graph = nx.read_edgelist('example_edge_list.txt', nodetype=int)
    print graph.edges(data=True)

    vis_input(graph)
    vis_post_output(graph, [3, 4, 5, 6], 'b')
    vis_post_output(graph, [1, 2, 3], 'g')
    vis_post_output(graph, [6, 7, 8], 'r')
