from itertools import combinations
import networkx as nx


# for comparision, this function will get the percolated k cliques
# in the most naive way, building all edges in the clique graph,
# before doing percolation.
def get_percolated_cliques(G, k):
    cliques = list(frozenset(c) for c in nx.find_cliques(G) if len(c) >= k)

    perc_graph = nx.Graph()
    for c1, c2 in combinations(cliques, 2):
        if len(c1.intersection(c2)) >= (k - 1):
            perc_graph.add_edge(c1, c2)

    for component in nx.connected_components(perc_graph):
        yield (frozenset.union(*component))
