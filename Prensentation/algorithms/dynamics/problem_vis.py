import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout

idx = 0


def vis_post_output(graph, node_dict, name):
    global idx

    color_dict = {1: 'red', 2: 'green', 3: 'blue', 4: 'yellow', 5: 'pink', 6: 'purple', 7: 'black', 8: 'orange',
                  9: 'cyan'}

    pos = nx.circular_layout(graph)
    nx.draw(graph, with_labels=True, pos=pos, font_size=20, node_size=2000, alpha=0.8, width=4,
            edge_color='grey', node_color='white')

    for node in node_dict:
        nx.draw_networkx_nodes(graph, pos=pos, nodelist=[node], node_size=2000, alpha=0.4,
                               node_color=color_dict[node_dict[node]])

    plt.axis('off')
    plt.savefig('./output_graph' + name + str(idx) + '.png', bbox_inches='tight', pad_inches=0,
                transparent=True)
    plt.show()
    idx += 1


if __name__ == '__main__':
    graph = nx.read_edgelist('example_edge_list.txt', nodetype=int)
    print graph.edges(data=True)

    # vis of async label propagation

    with open('async_lp_res0.txt') as ifs:
        dict_list = ifs.readlines()
        for my_dict in dict_list:
            vis_post_output(graph, eval(my_dict), '_async')

    with open('sync_lp_res0.txt') as ifs:
        dict_list = ifs.readlines()
        for my_dict in dict_list:
            vis_post_output(graph, eval(my_dict), '_sync')
