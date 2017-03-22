import networkx as nx
from networkx.readwrite import json_graph

import json

if __name__ == '__main__':
    graph = nx.read_edgelist('input/example_graph.edgelist', nodetype=int, data=(('weight', float),))
    assert isinstance(graph, nx.Graph)
    print 'edges:', graph.edges()

    # raw
    nx.write_adjlist(graph, 'output_raw/example_graph.adjlist')
    nx.write_multiline_adjlist(graph, 'output_raw/example_graph.multiline_adjlist')
    nx.write_edgelist(graph, 'output_raw/example_graph.edgelist')

    # better serialization
    nx.write_gpickle(graph, 'output_serialization/example_graph.pickle')
    nx.write_yaml(graph, 'output_serialization/example_graph.yaml')
    nx.write_graph6(graph, 'output_serialization/example_graph.graph6')

    # xml
    nx.write_gexf(graph, 'output_xml/example_graph.gexf')
    nx.write_graphml(graph, 'output_xml/example_graph.graphml')

    # json
    with open('output_json/node_link.json', 'w') as outfile:
        json.dump(json_graph.node_link_data(graph), outfile, indent=2)

    with open('output_json/adjacency.json', 'w') as outfile:
        json.dump(json_graph.adjacency_data(graph), outfile, indent=2)

    # other
    nx.write_gml(graph, 'output_other/example_graph.gml')
    nx.write_pajek(graph, 'output_other/example_graph.pajek')
