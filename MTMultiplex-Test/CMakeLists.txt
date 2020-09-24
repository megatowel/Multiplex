# CMakeList.txt : CMake project for MTMultiplexTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# We're building an executable.
add_executable (MTMultiplexTest)

# This looks nicer than putting everything in add_executable.
target_sources (MTMultiplexTest
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/MTMultiplexTest.cpp
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/MTMultiplexTest.h
)
target_include_directories (MTMultiplexTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../MTMultiplex/)
target_link_libraries (MTMultiplexTest PRIVATE MTMultiplex)