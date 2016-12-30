def get_sample_graph():
    with open('graph_eval_script.txt') as ifs:
        eval_line = ''.join(map(lambda ele: ele.strip(), ifs.readlines()))
    return eval(eval_line)


def compute_psis(N, t):
    psis = {N: 1.}
    for i in xrange(N - 1, 0, -1):
        psis[i] = psis[i + 1] * t / (float(i + 1.)) + 1.
    return psis
