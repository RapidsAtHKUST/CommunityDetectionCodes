from nx2gt import nx2gt
import networkx as nx
import graph_tool.all as gt
import math


def get_comm_dict_and_partition():
    comm_dict = {0: [0, 1, 2, 3, 7, 8, 12, 13, 14, 15, 17, 18, 19, 20, 21, 22, 23, 26, 27, 28, 29, 30, 31, 32, 33],
                 1: [24, 25, 31],
                 2: [0, 16, 4, 5, 6, 10]}
    partition_dict = {}
    for comm_id in comm_dict:
        for node in comm_dict[comm_id]:
            partition_dict[node] = comm_id
    print(partition_dict)
    return comm_dict, partition_dict


if __name__ == '__main__':
    g = nx2gt(nx.karate_club_graph())
    for v in g.vertices():
        print v
