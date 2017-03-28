from collections import defaultdict
import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout
import matplotlib.pyplot as plt


def get_percolated_cliques(G, k):
    def get_percolation_graph():
        percolation_graph = nx.Graph()
        cliques = [frozenset(c) for c in nx.find_cliques(G) if len(c) >= k]
        print 'first max cliques:', cliques

        percolation_graph.add_nodes_from(cliques)

        # First index which nodes are in which cliques
        membership_dict = defaultdict(list)
        for clique in cliques:
            for node in clique:
                membership_dict[node].append(clique)

        # For each clique, see which adjacent cliques percolate
        for clique in cliques:
            def get_adjacent_cliques(clique, membership_dict):
                adjacent_cliques = set()
                for n in clique:
                    for adj_clique in membership_dict[n]:
                        if clique != adj_clique:
                            adjacent_cliques.add(adj_clique)
                return adjacent_cliques

            for adj_clique in get_adjacent_cliques(clique, membership_dict):
                if len(clique.intersection(adj_clique)) >= (k - 1):
                    percolation_graph.add_edge(clique, adj_clique)

        print '\npercolation graph nodes:', percolation_graph.nodes()
        print 'percolation graph edges:', percolation_graph.edges()
        return percolation_graph

    percolation_graph = get_percolation_graph()

    pos = graphviz_layout(percolation_graph)
    comm_dict = {}
    # nx.draw(percolation_graph, , with_labels=True, node_size=500)
    #

    # Connected components of clique graph with perc edges
    # are the percolated cliques
    for idx, component in enumerate(nx.connected_components(percolation_graph)):
        print idx, component
        comm_dict[idx] = list(component)
        yield (frozenset.union(*component))

    color_list = ['r', 'g', 'b', 'y']

    # nodes
    for comm_id in comm_dict:
        nx.draw_networkx_nodes(percolation_graph, pos, nodelist=comm_dict[comm_id],
                               node_color=color_list[comm_id], node_size=2000, alpha=0.4, node_shape='h')

    node_label_dict = {}
    for node in percolation_graph.nodes():
        node_label_dict[node] = str(list(node))
    nx.draw_networkx_labels(percolation_graph, pos, labels=node_label_dict, font_size=15)

    edge_label_dict = {}
    for edge in percolation_graph.edges():
        edge_label_dict[edge] = str(list(frozenset.intersection(edge[0], edge[1])))

    nx.draw_networkx_edges(percolation_graph, pos, width=2, edge_color='grey')
    nx.draw_networkx_edge_labels(percolation_graph, pos, edge_labels=edge_label_dict, font_size=15)

    plt.axis('off')
    plt.get_current_fig_manager().window.showMaximized()
    # plt.savefig('./percolation_graph.pdf')
    plt.show()


if __name__ == '__main__':
    data_graph = nx.karate_club_graph()
    comm_list = list(get_percolated_cliques(data_graph, 3))

    print '\ncomm list:', comm_list
