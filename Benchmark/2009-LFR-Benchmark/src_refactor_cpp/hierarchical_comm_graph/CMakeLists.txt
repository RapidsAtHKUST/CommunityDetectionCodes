set(UtilFiles ../util/cast.cpp ../util/combinatorics.cpp ../util/histograms.cpp ../util/random.cpp ../util/cc.cpp)
#set(OtherUtilFiles ../util/deque_numeric.cpp)
add_executable(lfr_hierarchical_net benchm.cpp binary_benchm.cpp set_parameters.cpp ${UtilFiles})
target_compile_options(lfr_hierarchical_net PRIVATE -O3 -g)
