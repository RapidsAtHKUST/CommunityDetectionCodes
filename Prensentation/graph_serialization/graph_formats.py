import networkx as nx
from networkx.readwrite import json_graph
import json

if __name__ == '__main__':
    graph = nx.read_edgelist('input/example_graph.edgelist', nodetype=int, data=(('weight', float),))
    assert isinstance(graph, nx.Graph)
    print 'edges:', graph.edges()

    nx.write_adjlist(graph, 'concept_output/example_graph.adjlist')
    nx.write_multiline_adjlist(graph, 'concept_output/example_graph.multiline_adjlist')
    nx.write_edgelist(graph, 'concept_output/example_graph.edgelist')

    nx.write_gpickle(graph, 'bin_output/example_graph.pickle')

    nx.write_gexf(graph, 'xml_output/example_graph.gexf')
    nx.write_graphml(graph, 'xml_output/example_graph.graphml')

    nx.write_gml(graph, 'example_graph.gml')

    with open('json_output/node_link.json', 'w') as outfile:
        json.dump(json_graph.node_link_data(graph), outfile, indent=4)
