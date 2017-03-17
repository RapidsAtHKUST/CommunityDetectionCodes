cmake_minimum_required(VERSION 2.8)
project(2011-oslom)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -g")

find_package(Boost REQUIRED COMPONENTS)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(".")

FIND_PACKAGE(OpenMP REQUIRED)
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(UtilFiles
        program_options/set_parameters.cpp metric/mutual.cpp
        input_output/pretty_print.h input_output/tab_degree.cpp input_output/pajek.cpp input_output/partition.cpp

        common/cast.cpp common/combinatorics.cpp common/histograms.cpp common/random.cpp common/cc.cpp
        common/deque_numeric.cpp common/log_table.cpp

        collection/wsarray.cpp collection/module_collection.cpp

        graph/undirected/undirected_network.cpp graph/directed/directed_network.cpp
        graph/louvain_oslomnet.cpp graph/undirected/egocentric_undir.cpp graph/undirected/undir_weighted_tabdeg.cpp
        graph/oslom_net_global.cpp graph/directed/louvain_oslomnet_dir.cpp graph/directed/dir_weighted_tabdeg.cpp
        graph/directed/directed_oslomnet_evaluate.cpp graph/undirected/undirected_oslomnet_evaluate.cpp)

set(AlgoFiles algorithm/hierarchies.cpp)
add_executable(2011-oslom-dir algorithm/main_directed.cpp ${AlgoFiles} graph/directed/try_homeless_dir.cpp ${UtilFiles} ${AlgoFiles})
add_executable(2011-oslom-undir algorithm/main_undirected.cpp ${AlgoFiles} graph/undirected/try_homeless_undir.cpp ${UtilFiles} ${AlgoFiles})