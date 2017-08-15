set(UtilFiles ../util/cast.cpp ../util/combinatorics.cpp ../util/histograms.cpp ../util/random.cpp ../util/cc.cpp)

add_executable(lfr_dir_net benchm.cpp set_parameters.cpp ${UtilFiles})
target_compile_options(lfr_dir_net PRIVATE -O3 -g)
