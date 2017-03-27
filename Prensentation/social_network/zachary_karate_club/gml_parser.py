import networkx as nx

if __name__ == '__main__':
    graph = nx.read_gml('zachary_karate_club/original/karate_new.gml', label='id')
    print graph.edges()
