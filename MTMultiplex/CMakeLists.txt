# CMakeList.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
include(GenerateExportHeader)
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (MTMultiplex SHARED "MTMultiplex.cpp" "MTMultiplex.h")
target_sources (MTMultiplex
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mtmultiplex_export.h
)
include_directories(./include/msquic/)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/)
target_link_libraries (MTMultiplex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/msquic/msquic.lib)
# TODO: Add tests and install targets if needed.
generate_export_header(MTMultiplex)