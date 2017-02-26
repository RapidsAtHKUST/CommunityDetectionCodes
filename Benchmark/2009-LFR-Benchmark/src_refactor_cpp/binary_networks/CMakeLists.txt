set(UtilFiles ../util/cast.h ../util/cast.cpp ../util/combinatorics.h ../util/histograms.h ../util/random.h)
add_executable(lfm_bin_net src/benchm.cpp src/detail/set_parameters.cpp ${UtilFiles})