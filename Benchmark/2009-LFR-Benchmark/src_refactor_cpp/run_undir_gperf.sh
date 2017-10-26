mkdir -p build
cd build
cmake ..
make 
cd ..

build/undirected_graph/lfr_undir_net -N 30000 -k 80 -maxk 1000 -mu 0.1 -C 0.3

# convert profile.log to callgrind compatible format
pprof --callgrind build/undirected_graph/lfr_undir_net undir_net.log > undir_net_profile.callgrind
