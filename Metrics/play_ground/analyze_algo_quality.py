import networkx as nx
import re
from metrics.link_belong_modularity import *


def get_graph_info(file_path):
    def extract_first_two(collection):
        return [int(collection[0]), int(collection[1])]

    with open(file_path) as ifs:
        lines = map(lambda ele: ele.strip(), ifs.readlines())
        lines = filter(lambda ele: not ele.startswith('#') and re.match('.*[0-9]+.*[0-9]+', ele), lines)

        pair_list = map(lambda ele: extract_first_two(map(lambda ele2: ele2.strip(), ele.split())), lines)
        return nx.Graph(pair_list)


def get_community_result(file_path):
    def get_val(name, search_lines, delimiter=':'):
        return filter(lambda ele: ele.startswith(name), search_lines)[0].split(delimiter)[1]

    with open(file_path) as ifs:
        lines = ifs.readlines()
        comm_size = int(get_val('comm size', lines))
        name_res_str = get_val('name result', lines)
        whole_time_str = get_val('whole execution time', lines)
        comm_list = eval(name_res_str)

        for comm in comm_list:
            comm.sort()
        comm_list.sort(key=lambda comm: -len(comm))
        return comm_size, comm_list, whole_time_str


def print_comm(comm_list):
    for comm in comm_list:
        print comm


if __name__ == '__main__':
    graph = get_graph_info('../datasets/social_network/facebook_combined.txt')
    print 'num nodes:', graph.number_of_nodes(), 'num edges:', graph.number_of_edges()
    comm_num, comm_list, run_time = get_community_result('../src/cmake-build-debug/algorithm_demo/demon_fb.out')
    avg_comm_size = sum(map(lambda ele: len(ele), comm_list)) / comm_num
    print 'comm num:', comm_num, 'avg comm size:', avg_comm_size, 'whole execution time:', run_time
    print cal_modularity(graph, comm_list)
