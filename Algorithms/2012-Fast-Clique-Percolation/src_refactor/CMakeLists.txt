cmake_minimum_required(VERSION 2.8)
project(2012-fast-clique-percolation)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -g")

find_package(Boost REQUIRED COMPONENTS)
include_directories(${Boost_INCLUDE_DIRS})

FIND_PACKAGE(OpenMP REQUIRED)
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(GraphFiles graph/bloom.cpp graph/graph.cpp graph/loading.cpp graph/network.cpp graph/saving.cpp graph/strings.cpp graph/weights.cpp)
set(OtherFiles cliques.cpp cmdline.c percolateCliques.cpp)

add_executable(2012-fast-cpm ${GraphFiles} ${OtherFiles})