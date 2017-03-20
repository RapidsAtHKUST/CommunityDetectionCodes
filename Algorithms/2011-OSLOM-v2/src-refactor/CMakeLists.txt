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

set(GraphFiles
        graph/undirected/undirected_network.cpp graph/directed/directed_network.cpp
        graph/louvain_oslomnet.cpp graph/undirected/egocentric_undir.cpp graph/undirected/undir_weighted_tabdeg.cpp
        graph/oslom_net_global.cpp graph/directed/louvain_oslomnet_dir.cpp graph/directed/dir_weighted_tabdeg.cpp
        graph/directed/directed_oslomnet_evaluate.cpp graph/undirected/undirected_oslomnet_evaluate.cpp)

set(UtilFiles
        util/program_options/set_parameters.cpp util/metric/mutual.cpp
        util/input_output/pretty_print.h util/input_output/tab_degree.cpp util/input_output/pajek.cpp util/input_output/partition.cpp

        util/common/cast.cpp util/common/combinatorics.cpp util/common/histograms.cpp util/common/random.cpp util/common/cc.cpp
        util/common/deque_numeric.cpp util/common/log_table.cpp

        util/collection/wsarray.cpp util/collection/module_collection.cpp
        )

set(AlgoFiles algorithm/oslom/hierarchies.cpp)

add_executable(2011-oslom-dir algorithm/oslom/main_directed.cpp ${AlgoFiles} graph/directed/try_homeless_dir.cpp ${UtilFiles} ${GraphFiles} ${AlgoFiles})
add_executable(2011-oslom-undir algorithm/oslom/main_undirected.cpp ${AlgoFiles} graph/undirected/try_homeless_undir.cpp ${UtilFiles} ${GraphFiles} ${AlgoFiles})
add_executable(2011-oslom-visual visualization/main.cpp visualization/main_pajek.cpp)

add_executable(louvain_comm algorithm/louvain/community.cpp algorithm/louvain/graph.cpp algorithm/louvain/graph_binary.cpp algorithm/louvain/main_community.cpp)
add_executable(louvain_convert algorithm/louvain/community.cpp algorithm/louvain/graph.cpp algorithm/louvain/graph_binary.cpp algorithm/louvain/main_convert.cpp)
add_executable(louvain_hier algorithm/louvain/community.cpp algorithm/louvain/graph.cpp algorithm/louvain/graph_binary.cpp algorithm/louvain/main_hierarchy.cpp)

add_executable(infomap_undir algorithm/infomap/undir/Greedy.cc algorithm/infomap/undir/GreedyBase.cc algorithm/infomap/undir/infomap.cc algorithm/infomap/undir/Node.cc)
add_executable(infomap_dir algorithm/infomap/dir/Greedy.cc algorithm/infomap/dir/GreedyBase.cc algorithm/infomap/dir/infomap.cc algorithm/infomap/dir/Node.cc)