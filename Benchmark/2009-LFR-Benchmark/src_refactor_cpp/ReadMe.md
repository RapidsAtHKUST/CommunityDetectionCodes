# Description

There are five types of benchmark graph-input and ground-truth communities generators. The 5th generator 
gives hierarchical information, while others do not.

The code is organized as follows.

dir name | detail
--- | ---
[util](util) | some common utilities, io/metric/computation
[playground](playground) | not so useful, just for tests
[undirected_graph](undirected_graph) | undirected graph generator, 1st
[directed_graph](directed_graph) | directed graph generator, 2nd
[weighted_graph](weighted_graph) | undirected weighted graph generator, 3rd
[weighted_directed_graph](weighted_directed_graph) | directed weighted generator, 4th
[hierarchical_comm_graph](hierarchical_comm_graph) | graph generator, gives hierarchical ground-truth, 5th

## Parameters

- binary graph

```zsh
./lfr_bin_net -N 1000 -k 15 -maxk 50 -mu 0.1 -minc 20 -maxc 50
```

- directed graph

```zsh
./lfr_dir_net -N 1000 -k 15 -maxk 50 -mu 0.1 -minc 20 -maxc 50
```

- weighted graph

```zsh
./lfr_weighted_net -N 1000 -k 15 -maxk 50 -muw 0.1 -minc 20 -maxc 50
```

- weighted directed graph 

```zsh
./lfr_weighted_dir_net -N 1000 -k 15 -maxk 50 -muw 0.1 -minc 20 -maxc 50
```

- hierarchical comm ground truth graph

```zsh
./lfr_hierarchical_net -N 10000 -k 20 -maxk 50 -mu2 0.3 -minc 20 -maxc 50 -minC 100 -maxC 1000 -mu1 0.1
```

## Outputs

- first four: community.dat, network.dat, statistics.dat, time_seed.dat

- last one: community.dat, network.dat, statistics.dat, time_seed.dat, community_first_level.dat, community_second_level.dat