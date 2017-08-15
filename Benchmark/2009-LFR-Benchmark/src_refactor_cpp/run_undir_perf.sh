mkdir -p build
cd build
cmake ..
make 
cd ..
perf record --call-graph dwarf build/undirected_graph/lfr_undir_net -N 1000000 -k 15 -maxk 50 -mu 0.1 -minc 20 -maxc 50
