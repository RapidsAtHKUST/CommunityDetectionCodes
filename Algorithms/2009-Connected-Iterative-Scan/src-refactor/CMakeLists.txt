cmake_minimum_required(VERSION 2.8)
project(2009-cis)
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

set(IOFiles util/io_helper.cc util/string_helper.cc util/graph_io_helper.cc util/parameters_helper.cc)
set(GraphFiles graph/network.cc graph/temporal_network.cc)
set(SourceFiles ${IOFiles} ${GraphFiles} main.cc)
add_executable(2009-cis ${SourceFiles})
