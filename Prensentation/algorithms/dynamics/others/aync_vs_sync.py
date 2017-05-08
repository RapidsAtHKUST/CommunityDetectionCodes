# coding=utf8

import sys
import random
import networkx as nx
import matplotlib.pyplot as plt


# async lpa implementation from https://github.com/anatman-xx/lpa
# label-propagation algorithm
# use asynchronous updating for better results


def lpa(graph):
    """
    :type graph: nx.Graph
    """

    def estimate_stop_cond():
        for node in graph.nodes_iter():
            count = {}

            for neighbor in graph.neighbors_iter(node):
                neighbor_label = graph.node[neighbor]['label']
                neighbor_weight = graph.edge[node][neighbor]['weight']
                count[neighbor_label] = count.setdefault(neighbor_label, 0.0) + neighbor_weight

            # find out labels with maximum count
            count_items = count.items()
            count_items.sort(key=lambda x: x[1], reverse=True)

            # if there is not only one label with maximum count then choose one randomly
            labels = [k for k, v in count_items if v == count_items[0][1]]

            if graph.node[node]['label'] not in labels:
                return False

        return True

    loop_count = 0

    while True:
        loop_count += 1
        print 'loop', loop_count

        for node in graph.nodes_iter():
            count = {}

            for neighbor in graph.neighbors_iter(node):
                neighbor_label = graph.node[neighbor]['label']
                neighbor_weight = graph.edge[node][neighbor]['weight']
                count[neighbor_label] = count.setdefault(neighbor_label, 0.0) + neighbor_weight

            # find out labels with maximum count
            count_items = count.items()
            count_items.sort(key=lambda x: x[1], reverse=True)

            # if there is not only one label with maximum count then choose one randomly
            labels = [(k, v) for k, v in count_items if v == count_items[0][1]]
            label = random.sample(labels, 1)[0][0]

            graph.node[node]['label'] = label

        if estimate_stop_cond() is True or loop_count >= 10:
            print 'complete'
            return
