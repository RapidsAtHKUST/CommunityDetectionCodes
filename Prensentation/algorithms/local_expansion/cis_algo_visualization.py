import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout
import matplotlib.pyplot as plt

seed_vertex_str = 'seed vertex'
intermediate_result_str = 'intermediate result'
current_result_community_str = 'current result community'
iter_num_str = 'iter num'


def get_cis_info():
    with open('cis_algo_res.txt') as ifs:
        blocks = ''.join(ifs.readlines()).split('\n\n')

    intermediate_res_block = blocks[:-6]
    name_dict = dict(eval(blocks[-6].split(':')[1]))
    print 'name dict:', name_dict, '\n'

    def block2dict(block_str):
        def eval_pair(pair_str):
            key, val = tuple(pair_str.split(':'))
            return key, eval(val)

        return dict(map(eval_pair, block_str.split('\n')))

    iteration_dict_list = map(block2dict, intermediate_res_block)

    for dict_item in iteration_dict_list:
        dict_item[iter_num_str] = dict_item[seed_vertex_str]
        dict_item[intermediate_result_str] = sorted(map(lambda ele: name_dict[ele], dict_item[intermediate_result_str]))
        dict_item[current_result_community_str] = map(lambda comm: sorted(map(lambda ele: name_dict[ele], comm)),
                                                      dict_item[current_result_community_str])
        dict_item[seed_vertex_str] = name_dict[dict_item[seed_vertex_str]]
    return iteration_dict_list


def draw_local_expansion_graph(graph, iter_info, color_str):
    pos = graphviz_layout(graph)
    nx.draw(graph, pos, width=4.0, alpha=0.5, edge_color='grey',
            node_color='white', node_size=500, with_labels=True)

    nx.draw_networkx_nodes(graph, pos, nodelist=iter_info[intermediate_result_str], node_size=500, node_color=color_str,
                           alpha=0.4)
    nx.draw_networkx_nodes(graph, pos, nodelist=[iter_info[seed_vertex_str]], node_size=500, node_color=color_str,
                           alpha=0.6)
    plt.axis('off')
    plt.savefig('./iter_info' + str(iter_info[iter_num_str]) + '.pdf', bbox_inches='tight', pad_inches=0)
    plt.show()


def draw_global_result_graph(graph, first_node_list, newly_added_node_list, color_str):
    pos = graphviz_layout(graph)
    nx.draw(graph, pos, width=4.0, alpha=0.5, edge_color='grey',
            node_color='white', node_size=500, with_labels=True)

    nx.draw_networkx_nodes(graph, pos, nodelist=first_node_list, node_size=500, node_color=color_str,
                           alpha=0.4)
    nx.draw_networkx_nodes(graph, pos, nodelist=newly_added_node_list, node_size=500, node_color=color_str,
                           alpha=0.6)
    plt.axis('off')
    plt.savefig('cis_global_result_graph.pdf', bbox_inches='tight', pad_inches=0)
    plt.savefig('cis_global_result_graph.png', bbox_inches='tight', pad_inches=0)
    plt.show()


if __name__ == '__main__':
    graph = nx.karate_club_graph()
    iter_info_list = get_cis_info()
    for i in range(2):
        print iter_info_list[i]
    iter0_node_list = iter_info_list[0][current_result_community_str][0]
    iter1_node_list = sorted(set(iter_info_list[1][current_result_community_str][0]) - set(iter0_node_list))
    print iter0_node_list
    print iter1_node_list
    draw_local_expansion_graph(graph, iter_info_list[0], 'r')
    draw_local_expansion_graph(graph, iter_info_list[1], 'b')
    draw_global_result_graph(graph, iter0_node_list, iter1_node_list, 'purple')
