from main_files.util_helper import *

if __name__ == '__main__':
    adj_list_dict = get_sample_graph()
    edge_num = get_edge_num(adj_list_dict)
    print 'node num:', len(adj_list_dict)
    print 'edge num:', edge_num
