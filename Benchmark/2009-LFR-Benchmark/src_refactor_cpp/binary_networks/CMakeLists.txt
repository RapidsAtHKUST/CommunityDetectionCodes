set(UtilFiles ../util/cast.cpp ../util/combinatorics.cpp ../util/histograms.cpp ../util/random.cpp)
add_executable(lfr_bin_net benchm.cpp set_parameters.cpp ${UtilFiles})