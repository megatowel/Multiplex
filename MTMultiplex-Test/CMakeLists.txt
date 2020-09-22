# CMakeList.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (MTMultiplexTest "MTMultiplexTest.cpp" "MTMultiplexTest.h")
target_sources (MTMultiplexTest
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mtmultiplex.h
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mtmultiplex_export.h
)
target_link_libraries (MTMultiplexTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/MTMultiplex.lib)