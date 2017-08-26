import os


def generate_different_avg(vertex_num):
    cluster_coefficient = 0.4
    max_degree = 100
    avg_degree_lst = [i * 10 for i in xrange(1, 8)]

    for avg_degree in avg_degree_lst:
        param_lst = map(str, ['build/undirected_graph/lfr_undir_net', '-N', vertex_num, '-k', avg_degree, '-maxk',
                              max_degree, '-mu', 0.1, '-C', cluster_coefficient])
        os.system(' '.join(param_lst))
        dir_name = 'output_vary_deg' + os.sep + '_'.join(
            map(str, ['v', vertex_num, 'avgk', avg_degree, 'maxk', max_degree, 'C', cluster_coefficient]))
        os.system('mkdir -p ' + dir_name)
        os.system('mv *.dat ' + dir_name)


def generate_different_coefficient(vertex_num):
    cluster_coefficient_lst = [i * 0.1 for i in xrange(1, 8)]
    avg_degree = 40
    max_degree = 100

    for cluster_coefficient in cluster_coefficient_lst:
        param_lst = map(str, ['build/undirected_graph/lfr_undir_net', '-N', vertex_num, '-k', avg_degree, '-maxk',
                              max_degree, '-mu', 0.1, '-C', cluster_coefficient])
        os.system(' '.join(param_lst))
        dir_name = 'output_vary_coefficient' + os.sep + '_'.join(
            map(str, ['v', vertex_num, 'avgk', avg_degree, 'maxk', max_degree, 'C', cluster_coefficient]))
        os.system('mkdir -p ' + dir_name)
        os.system('mv *.dat ' + dir_name)


if __name__ == '__main__':
    generate_different_avg(vertex_num=10000000)
    generate_different_coefficient(vertex_num=10000000)
