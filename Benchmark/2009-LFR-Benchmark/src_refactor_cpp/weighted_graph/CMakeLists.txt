set(UtilFiles ../util/cast.cpp ../util/combinatorics.cpp ../util/histograms.cpp ../util/random.cpp ../util/cc.cpp)
add_executable(lfr_weighted_net benchm.cpp binary_benchm.cpp set_parameters.cpp ${UtilFiles} )
target_compile_options(lfr_weighted_net PRIVATE -O3 -g)