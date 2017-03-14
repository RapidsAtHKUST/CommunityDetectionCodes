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

set(UtilFiles util/common/cast.cpp util/common/combinatorics.cpp util/common/histograms.cpp util/common/random.cpp util/common/cc.cpp
        util/input_output/tab_degree.cpp
        util/metric/mutual.cpp
        util/graph/undirected_network.cpp
        util/collection/wsarray.cpp util/collection/module_collection.cpp)
set(AlgoFiles oslom_net_check_overlap.cpp oslom_net_unions.cpp)
add_executable(2011-oslom-dir main_body.cpp main_directed.cpp ${AlgoFiles} try_homeless_dir.cpp ${UtilFiles})
add_executable(2011-oslom-undir main_body.cpp main_undirected.cpp ${AlgoFiles} try_homeless_undir.cpp ${UtilFiles})