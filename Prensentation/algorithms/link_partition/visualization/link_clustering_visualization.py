import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout
import matplotlib.pyplot as plt


def get_sorted_pair(a, b):
    return tuple(sorted([a, b]))


def get_link_graph():
    graph = nx.karate_club_graph()
    edges = graph.edges()

    new_graph = nx.Graph()
    new_graph.add_nodes_from(edges)
    new_graph_edge_list = []

    for edge in edges:
        for inner_loop_edge in edges:
            if edge is not inner_loop_edge and len(set(edge).intersection(set(inner_loop_edge))) > 0:
                new_graph_edge_list.append(get_sorted_pair(edge, inner_loop_edge))
    print 'link graph edge list:', new_graph_edge_list
    new_graph.add_edges_from(new_graph_edge_list)
    return new_graph


def draw_link_graph(graph):
    pos = graphviz_layout(graph)
    nx.draw(graph, pos, width=4.0, alpha=0.5, edge_color='grey', node_size=500, node_shape='h', with_labels=True)

    edge_dict = {}
    for edge in graph.edges():
        edge_dict[edge] = str(list(set(edge[0]).intersection(set(edge[1]))))

    # nx.draw_networkx_edge_labels(graph, pos,edge_labels=edge_dict, font_size=6)
    plt.axis('off')
    plt.savefig('./link_graph.pdf', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.savefig('./link_graph.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


def draw_partitioned_link_graph(graph, comm_list):
    pos = graphviz_layout(graph)
    nx.draw(graph, pos, width=4.0, alpha=0.5, edge_color='grey',
            node_color='white', node_size=500, node_shape='h', with_labels=True)

    color_list = ['red', 'green', 'blue', 'yellow', 'pink', 'purple', 'black', 'orange']
    for idx, comm in enumerate(comm_list):
        print idx, comm
        nx.draw_networkx_nodes(graph, pos, nodelist=comm, node_color=color_list[idx],
                               node_size=500, node_shape='h', alpha=0.5)

    edge_dict = {}
    for edge in graph.edges():
        edge_dict[edge] = str(list(set(edge[0]).intersection(set(edge[1]))))

    # nx.draw_networkx_edge_labels(graph, pos,edge_labels=edge_dict, font_size=6)
    plt.axis('off')
    plt.savefig('./link_partition_graph.pdf', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.savefig('./link_partition_graph.png', bbox_inches='tight', pad_inches=0, transparent=True)
    plt.show()


def get_link_partition_res():
    min_size = 3
    with open('../result/karate_edge_list_maxS0.333333_maxD0.284758.comm2edges.txt') as ifs:
        comm_list = map(lambda line: line.strip().split(), ifs.readlines())
        filtered_comm_list = filter(lambda comm: len(comm) > min_size, comm_list)
        filtered_comm_list = map(lambda comm: map(lambda edge: tuple(sorted(map(int, edge.split(',')))), comm[1:]),
                                 filtered_comm_list)

    for comm in filtered_comm_list:
        print 'link comm:', comm
    print 'link comm len:', len(filtered_comm_list)
    return filtered_comm_list


def get_overlapping_community_detection_res(link_partition):
    community_detection_res = map(lambda ele: sorted(reduce(lambda l, r: set(l) | set(r), ele, set())), link_partition)
    print
    for comm in community_detection_res:
        print 'node comm:', comm
    # do some postprocessing, merging similar comms
    return community_detection_res


if __name__ == '__main__':
    link_graph = get_link_graph()
    link_partition = get_link_partition_res()
    overlap_res = get_overlapping_community_detection_res(link_partition)

    print link_graph.nodes()
    draw_link_graph(link_graph)
    draw_partitioned_link_graph(link_graph, link_partition)
