# from http://stackoverflow.com/questions/22070196/community-detection-in-networkx

import community  # --> http://perso.crans.org/aynaud/communities/
import networkx as nx


def get_comm_dict_and_partition(g):
    partition = community.best_partition(g)
    print "Louvain Modularity: ", community.modularity(partition, g)
    print "Louvain Partition: ", partition

    reverse_dict = {}
    for node in partition:
        if partition[node] not in reverse_dict:
            reverse_dict[partition[node]] = []
        reverse_dict[partition[node]].append(node)
    print 'Node List Dict:', reverse_dict
    return reverse_dict, partition


if __name__ == '__main__':
    g = nx.karate_club_graph()
    reverse_dict, partition = get_comm_dict_and_partition(g)
    for comm in reverse_dict:
        print comm, ':', reverse_dict[comm]
