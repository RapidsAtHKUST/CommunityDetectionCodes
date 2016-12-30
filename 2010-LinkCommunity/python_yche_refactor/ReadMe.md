#LinkComm
##Current Status
Only support undirected graph(weighted or unweighted are all supported), which could be found in the implementation of similarity calculation in [link_clustering_algo.py](link_clustering_algo.py).

```python
def similarities_weighted(adj_dict, edge_weight_dict):
    i_adj = dict((n, adj_dict[n] | {n}) for n in adj_dict)
    Aij = copy(edge_weight_dict)
    n2a_sqrd = {}
    for n in adj_dict:
        Aij[n, n] = 1.0 * sum(edge_weight_dict[get_sorted_pair(n, i)] for i in adj_dict[n]) / len(adj_dict[n])
        n2a_sqrd[n] = sum(Aij[get_sorted_pair(n, i)] ** 2 for i in i_adj[n])  # includes (n,n)!

    min_heap = []
    for ind, n in enumerate(adj_dict):
        if len(adj_dict[n]) > 1:
            for i, j in combinations(adj_dict[n], 2):
                edge_pair = get_sorted_pair(get_sorted_pair(i, n), get_sorted_pair(j, n))
                inc_ns_i, inc_ns_j = i_adj[i], i_adj[j]

                ai_dot_aj = 1.0 * sum(
                    Aij[get_sorted_pair(i, x)] * Aij[get_sorted_pair(j, x)] for x in inc_ns_i & inc_ns_j)

                S = ai_dot_aj / (n2a_sqrd[i] + n2a_sqrd[j] - ai_dot_aj)  # tanimoto similarity
                heappush(min_heap, (1 - S, edge_pair))
    return [heappop(min_heap) for i in xrange(len(min_heap))]  # return ordered edge pairs
```

##Algorithm Heuristic
- First compuate edge similarities once
- Second hierarchically merge communities