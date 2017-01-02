#!/usr/bin/python
import networkx as nx
from datetime import datetime
import sys
from collections import defaultdict


# this method uses the nodesToCliques dictionary, in order to only test cliques for intersection,
# if the cliques overlap by more than 1 node.
# Even if this improved method is used to build the full clique graph,
# it is still prohibitively slow, as clique graphs are often extremely large in practice.
def get_adjacent_cliques(clique, membership_dict):
    adjacent_cliques = set()
    for n in clique:
        for adj_clique in membership_dict[n]:
            if clique != adj_clique:
                adjacent_cliques.add(adj_clique)
    return adjacent_cliques


# This method builds only a partial set of the edges in the clique graph, as it
def get_fast_percolated_cliques(G, k):
    print >> sys.stderr, "Starting Clique Finding. Time: ", str(datetime.time(datetime.now()))
    cliques = [frozenset(c) for c in nx.find_cliques(G) if len(c) >= k]

    # randomly sampling cliques leads to a rapid falloff in NMI of results;
    # this naive stochastic method is not appropriate.
    # randomCliques = random.sample(cliques, (len(cliques) - len(cliques)/5))
    # cliques = randomCliques

    print >> sys.stderr, "Cliques found. Time: ", str(datetime.time(datetime.now()))
    nodesToCliquesDict = defaultdict(list)
    for clique in cliques:
        for node in clique:
            nodesToCliquesDict[node].append(clique)

    print >> sys.stderr, "NodesToCliques Map built. Time: ", str(datetime.time(datetime.now()))

    cliquesToComponents = dict()
    currentComponent = 0

    cliquesProcessed = 0
    for clique in cliques:
        cliquesProcessed += 1
        if cliquesProcessed % 10000 == 0:
            print >> sys.stderr, "Total cliques processed: ", str(cliquesProcessed), " time: ", str(
                datetime.time(datetime.now()))

        if not clique in cliquesToComponents:
            currentComponent += 1
            cliquesToComponents[clique] = currentComponent
            frontier = set()
            frontier.add(clique)
            componentCliquesProcessed = 0

            while len(frontier) > 0:
                # remove from nodes->cliques map
                # for each adjacent clique, if it percolates, add it to the frontier, and number it
                currentClique = frontier.pop()
                componentCliquesProcessed += 1
                if componentCliquesProcessed % 1000 == 0:
                    print >> sys.stderr, "Component cliques processed: ", str(
                        componentCliquesProcessed), " time: ", str(datetime.time(datetime.now()))
                    print >> sys.stderr, "Size of frontier: ", len(frontier)

                for neighbour in get_adjacent_cliques(currentClique, nodesToCliquesDict):
                    # this does not get appreciably faster by counting intersection size,
                    #  while enumerating neighbours, in the C++ implementation this actually slightly slows performance.
                    if len(currentClique.intersection(neighbour)) >= (k - 1):
                        # add to current component
                        cliquesToComponents[neighbour] = currentComponent
                        # add to the frontier
                        frontier.add(neighbour)
                        for node in neighbour:
                            nodesToCliquesDict[node].remove(neighbour)

    print >> sys.stderr, "CliqueGraphComponent Built. Time: ", str(datetime.time(datetime.now()))

    # get the union of the nodes in each component.
    # print "Number of components:" , currentComponent
    componentToNodes = defaultdict(set)
    for clique in cliquesToComponents:
        componentCliqueIn = cliquesToComponents[clique]
        componentToNodes[componentCliqueIn].update(clique)

    print >> sys.stderr, "Node Components Assigned. Time: ", str(datetime.time(datetime.now()))
    return componentToNodes.values()


def print_sorted_res(result):
    sortedCliques = [sorted(list(x)) for x in result]
    sortedCliquesAsStrings = sorted([(" ".join([str(x) for x in clique])) for clique in sortedCliques])
    for c in sortedCliquesAsStrings:
        print c


if __name__ == '__main__':
    print >> sys.stderr, "Loading Graph. Time: ", str(datetime.time(datetime.now()))
    G = nx.read_edgelist(sys.argv[1], nodetype=int, delimiter="\t")
    k = int(sys.argv[2])

    print >> sys.stderr, "Graph Loaded. Time: ", str(datetime.time(datetime.now()))

    res = get_fast_percolated_cliques(G, k)
    with open(sys.argv[3], 'w') as ofs:
        for c in res:
            # print " ".join([str(x) for x in c])
            ofs.write(" ".join([str(x) for x in c]))
            ofs.write("\n")

    print_sorted_res(res)
