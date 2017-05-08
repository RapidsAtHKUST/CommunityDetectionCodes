import networkx as nx
from iteration_utilities import flatten
import random

w_in = 0
w_all = 0

if __name__ == '__main__':
    data_graph = nx.read_edgelist('example_edge_list.txt', nodetype=int)
    print data_graph.edges()
    print nx.subgraph(data_graph, [1, 2, 3]).edges()
    print
    members = {2}


    def fitness(new_members, is_print=False):
        if len(new_members) == 1:
            return 0
        else:
            new_nodes = set(flatten(map(lambda mem: nx.neighbors(data_graph, mem), new_members))) | new_members
            global w_in
            global w_all
            w_all = len(nx.subgraph(data_graph, new_nodes).edges())
            w_in = len(nx.subgraph(data_graph, new_members).edges())
            if is_print:
                print 'w_in', w_in, nx.subgraph(data_graph, new_members).edges()
                print 'w_all', w_all, nx.subgraph(data_graph, new_nodes).edges()
            return float(w_in) / w_all


    def expand(seed_set):
        members = seed_set
        print 'seed:', members
        is_change = True
        while is_change:
            to_check_neighbors = list(flatten(map(lambda mem: nx.neighbors(data_graph, mem), members)))
            random.shuffle(to_check_neighbors)
            print to_check_neighbors
            is_change = False
            # for neighbor in to_check_neighbors:
            for neighbor in to_check_neighbors:
                if fitness(members | {neighbor}) > fitness(members):
                    is_change = True
                    members.add(neighbor)
                    fitness(members, is_print=True)
                    print 'add neighbor:', neighbor, members, 'w_in:', w_in, 'w_all:', w_all
                    break

            for member in members:
                if fitness(members - {member}) > fitness(members):
                    is_change = True
                    members.remove(member)
                    fitness(members)
                    print 'remove member:', member, members, 'w_in', w_in, 'w_all:', w_all
                    break
        print set(members)
        print '\n----------------------------\n'


    for node in data_graph.nodes():
        expand({node})
