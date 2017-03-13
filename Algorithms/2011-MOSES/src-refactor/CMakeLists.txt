cmake_minimum_required(VERSION 2.8)
project(2011-moses)
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

set(AlgorithmFiles algorithm/moses.cpp algorithm/overlapping.cpp)
set(GraphUtilFiles util/graph/graph_loading.cpp util/graph/graph_representation.cpp algorithm/grouping.cpp)
set(OtherUtilFiles util/aaron_utils.cpp util/range.cpp)
set(SourceFiles ${OtherUtilFiles} ${GraphUtilFiles} ${AlgorithmFiles} ${Others})

add_executable(2011-moses ${SourceFiles} ${Headers})
#add_executable(2011-moses-group-info algorithm/group_status_exec.cpp ${SourceFiles} ${Headers})
