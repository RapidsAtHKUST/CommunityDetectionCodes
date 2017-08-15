mkdir -p build
cd build
cmake ..
make 
cd ..

build/undirected_graph/lfr_undir_net -N 1000000 -k 15 -maxk 50 -mu 0.1 -minc 20 -maxc 50

# convert profile.log to callgrind compatible format
pprof --callgrind build/undirected_graph/lfr_undir_net undir_net.log > undir_net_profile.callgrind
