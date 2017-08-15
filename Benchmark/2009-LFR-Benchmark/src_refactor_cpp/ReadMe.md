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

## Performance

### Build & Profiling

change corresponding [undirected_graph/CMakeLists.txt](undirected_graph/CMakeLists.txt) and enable `gperf`.

build and profiling scripts: see [run_undir_gperf.sh](run_undir_gperf.sh) and [run_undir_perf.sh](run_undir_perf.sh).

### Some Experiments

undirected graph, 1 million vertices, average degree 15, 166s

```zsh
**************************************************************
number of nodes:	1000000
average degree:	15
maximum degree:	50
exponent for the degree distribution:	2
exponent for the community size distribution:	1
mixing parameter:	0.1
number of overlapping nodes:	0
number of memberships of the overlapping nodes:	0
community size range set equal to [20 , 50]
**************************************************************

with google perf start------------
building communities...
connecting communities...
recording network...


---------------------------------------------------------------------------
network of 1000000 vertices and 7651733 edges;	 average degree = 15.3035

average mixing parameter: 0.100166 +/- 0.0390085
p_in: 0.428117	p_out: 1.46288e-06


with google perf end--------------
PROFILE: interrupts/evictions/bytes = 16624/5766/629824
total time:166659 ms
```

## Parameters

- undirected graph

```zsh
build/undirected_graph/lfr_undir_net -N 1000 -k 15 -maxk 50 -mu 0.1 -minc 20 -maxc 50
```

- directed graph

```zsh
build/directed_graph/lfr_dir_net -N 1000 -k 15 -maxk 50 -mu 0.1 -minc 20 -maxc 50
```

- weighted graph

```zsh
build/weighted_graph/lfr_weighted_net -N 1000 -k 15 -maxk 50 -muw 0.1 -minc 20 -maxc 50
```

- weighted directed graph

```zsh
build/weighted_directed_graph/lfr_weighted_dir_net -N 1000 -k 15 -maxk 50 -muw 0.1 -minc 20 -maxc 50
```

- hierarchical comm ground truth graph

```zsh
build/hierarchical_comm_graph/lfr_hierarchical_net -N 10000 -k 20 -maxk 50 -mu2 0.3 -minc 20 -maxc 50 -minC 100 -maxC 1000 -mu1 0.1
```

## Outputs

- first four: community.dat, network.dat, statistics.dat, time_seed.dat

- last one: community.dat, network.dat, statistics.dat, time_seed.dat, community_first_level.dat, community_second_level.dat
