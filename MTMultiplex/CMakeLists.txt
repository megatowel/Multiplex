# CMakeList.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (MTMultiplex SHARED)

# This looks nicer than putting everything in add_library.
target_sources (MTMultiplex
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/MTMultiplex.cpp
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/MTMultiplex.h
)

find_path(ENET_INCLUDE_DIR enet/enet.h)
message("enet include: ${ENET_INCLUDE_DIR}")
find_library(ENET_LIBRARY enet)
message("enet library: ${ENET_LIBRARY}")
target_include_directories(MTMultiplex PRIVATE ${ENET_INCLUDE_DIR})
link_libraries(MTMultiplex PRIVATE ${ENET_LIBRARY})

target_link_libraries (MTMultiplex PRIVATE enet)
