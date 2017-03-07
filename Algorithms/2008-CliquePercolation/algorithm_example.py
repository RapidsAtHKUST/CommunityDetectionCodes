import networkx as nx

from conradlee_clique_percolation import *

if __name__ == '__main__':
    with open('example_edge_list.txt', 'r') as ifs:
        edge_list = map(lambda ele: ele.strip().split(), ifs.readlines())
    data_graph = nx.Graph(edge_list)
    comm_list = list(get_percolated_cliques(data_graph, 3))
    print comm_list
