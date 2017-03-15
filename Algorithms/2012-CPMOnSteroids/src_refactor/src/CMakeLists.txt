cmake_minimum_required(VERSION 2.8)
project(2012-ParCPM)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -g")

find_package(Boost REQUIRED COMPONENTS)
include_directories(${Boost_INCLUDE_DIRS})
set(IgraphPath ../../../../SubModules/igraph/include/)
include_directories(${IgraphPath})

FIND_PACKAGE(OpenMP REQUIRED)
if (OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(SOURCE_FILES  cliques.c cos.c
        cospoc.c dsforest.c main.c matrix.c output_communities.c)

add_executable(2012-ParCPM ${SOURCE_FILES})
target_link_libraries(2012-ParCPM igraph)
target_link_libraries(2012-ParCPM m)

add_executable(max-clique extras/maximal_cliques.c)
target_link_libraries(max-clique igraph)
