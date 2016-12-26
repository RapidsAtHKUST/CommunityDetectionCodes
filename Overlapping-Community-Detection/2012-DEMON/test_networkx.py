import networkx as nx

g = nx.Graph()
fin = open("/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/karate_edges_input.csv")
for l in fin:
    l = l.rstrip().split(" ")
    print  l[0] + ',' + l[1];
    g.add_edge(l[0], l[1])

print '\n'

# Iterate Through Vertices
for node in nx.nodes(g):
    print node

# print len(nx.nodes(g))

print 'nodes_num:' + str(g.number_of_nodes())
