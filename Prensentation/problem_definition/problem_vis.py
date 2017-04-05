import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout


def vis_input(graph):
    pos = graphviz_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=16, node_size=600, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    edge_dict = {}
    for edge in graph.edges():
        edge_dict[edge] = graph[edge[0]][edge[1]]['w']

    nx.draw_networkx_edge_labels(graph, pos, edge_labels=edge_dict, font_size=14, alpha=0.1, font_color='blue')
    plt.axis('off')
    plt.savefig('./input_graph.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


def vis_output(graph, comm, comm_color, idx):
    pos = graphviz_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=16, node_size=600, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    nx.draw_networkx_nodes(graph, with_labels=True, pos=pos, font_size=16, node_size=600, alpha=0.8, width=4,
                           node_color=comm_color, nodelist=comm)

    edge_dict = {}
    for edge in graph.edges():
        edge_dict[edge] = graph[edge[0]][edge[1]]['w']

    nx.draw_networkx_edge_labels(graph, pos, edge_labels=edge_dict, font_size=14, alpha=0.1, font_color='blue')
    plt.axis('off')
    plt.savefig('./output_graph' + str(idx) + '.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


def vis_post_output(graph, comm_list, color_list):
    pos = graphviz_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=16, node_size=600, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    for idx, comm in enumerate(comm_list):
        nx.draw_networkx_nodes(graph, with_labels=True, pos=pos, font_size=16, node_size=600, alpha=0.8, width=4,
                               node_color=color_list[idx], nodelist=comm)

    edge_dict = {}
    for edge in graph.edges():
        edge_dict[edge] = graph[edge[0]][edge[1]]['w']

    nx.draw_networkx_edge_labels(graph, pos, edge_labels=edge_dict, font_size=14, alpha=0.1, font_color='blue')
    plt.axis('off')
    plt.savefig('./output_post_graph' + '.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


if __name__ == '__main__':
    graph = nx.read_edgelist('demo_input_graph.txt', nodetype=int, data=(('w', float),))
    print graph.edges(data=True)

    vis_input(graph)

    with open('demo_cis_output_result.txt') as ifs:
        eval_str = ifs.readline()
        comm_list = eval(eval_str)
        color_list = ['r', 'b', 'magenta']
        for idx, comm in enumerate(comm_list):
            print comm, idx
            vis_output(graph, comm, color_list[idx], idx)
        comm_list.append([1])
        vis_post_output(graph, comm_list, color_list)
