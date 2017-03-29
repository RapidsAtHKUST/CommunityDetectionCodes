import networkx as nx

if __name__ == '__main__':
    graph = nx.karate_club_graph()
    lines = map(lambda edge: str(edge[0]) + '\t' + str(edge[1]), graph.edges())
    print lines
    with open('karate_edge_list.txt', 'w') as ofs:
        ofs.write('\n'.join(lines))
