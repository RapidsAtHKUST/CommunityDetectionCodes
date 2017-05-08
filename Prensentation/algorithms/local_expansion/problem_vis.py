import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout

idx = 0


def vis_input(graph):
    # pos = graphviz_layout(graph)
    pos = nx.circular_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=20, node_size=2000, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    plt.axis('off')
    plt.savefig('./demo_graph.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


def vis_post_output(graph, intra_comm, all_local_structure, color, nodes):
    global idx
    iter_comm = list(set(all_local_structure) - set(intra_comm))
    pos = nx.circular_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=20, node_size=2000, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    nx.draw_networkx_nodes(graph, pos=pos, nodelist=nodes, node_size=2000, alpha=0.4, node_color=color)

    nx.draw_networkx_edges(graph, pos=pos, edgelist=iter_comm, alpha=0.8, width=4, edge_color='black')
    nx.draw_networkx_edges(graph, pos=pos, edgelist=intra_comm, alpha=0.8, width=4, edge_color=color)

    plt.axis('off')
    plt.savefig('./output_post_graph' + str(idx) + '.png', bbox_inches='tight', pad_inches=0,
                transparent=True)
    plt.show()
    idx += 1


if __name__ == '__main__':
    graph = nx.read_edgelist('example_edge_list.txt', nodetype=int)
    print graph.edges(data=True)

    vis_input(graph)
    # vis of vertex 5
    vis_post_output(graph, [], [(3, 4), (3, 5), (4, 5), (4, 6), (5, 6)], 'b', [5])
    vis_post_output(graph, [(4, 5)], [(3, 4), (3, 5), (4, 5), (4, 6), (5, 6)], 'b', [4, 5])
    vis_post_output(graph, [(3, 4), (3, 5), (4, 5)], [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5), (4, 6), (5, 6)],
                    'b', [3, 4, 5])
    vis_post_output(graph, [(3, 4), (3, 5), (4, 5), (4, 6), (5, 6)],
                    [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5), (4, 6), (5, 6), (6, 8), (6, 7), (7, 8)], 'b',
                    [3, 4, 5, 6])
    vis_post_output(graph, [(1, 3), (3, 4), (3, 5), (4, 5), (4, 6), (5, 6)],
                    [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5), (4, 6), (5, 6), (6, 8), (6, 7), (7, 8)], 'b',
                    [1, 3, 4, 5, 6])
    vis_post_output(graph, [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5), (4, 6), (5, 6)],
                    [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5), (4, 6), (5, 6), (6, 8), (6, 7), (7, 8)], 'b',
                    [1, 2, 3, 4, 5, 6])
    vis_post_output(graph, [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5)],
                    [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5), (4, 6), (5, 6)], 'b', [1, 2, 3, 4, 5])

    # final vis of vertex 1
    vis_post_output(graph, [(1, 2), (1, 3), (2, 3)], [(1, 2), (1, 3), (2, 3), (3, 4), (3, 5), (4, 5)], 'g', [1, 2, 3])

    # final vis of vertex 8
    vis_post_output(graph, [(4, 5), (4, 6), (5, 6), (6, 8), (6, 7), (7, 8), (7, 9)],
                    [(3, 4), (3, 5), (4, 5), (4, 6), (5, 6), (6, 8), (6, 7), (7, 8), (7, 9)], 'r', [4, 5, 6, 7, 8, 9])
