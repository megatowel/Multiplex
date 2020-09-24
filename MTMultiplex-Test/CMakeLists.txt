# CMakeList.txt : CMake project for MTMultiplexTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# We're building an executable.
add_executable (MTMultiplexTest MTMultiplexTest.cpp MTMultiplexTest.h)

target_include_directories (MTMultiplexTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../MTMultiplex/)
target_link_libraries (MTMultiplexTest PRIVATE MTMultiplex)

add_custom_command(TARGET MTMultiplexTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_BINARY_DIR}/MTMultiplex/MTMultiplex.dll" "${PROJECT_BINARY_DIR}/MTMultiplex-Test/")