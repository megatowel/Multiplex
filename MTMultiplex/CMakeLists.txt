# CMakeList.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.16)

# Add source to this project's executable.
add_library (MTMultiplex SHARED MTMultiplex.cpp "MTMultiplex.h")

find_path(ENET_INCLUDE_DIR enet/enet.h)
message("-- enet include: ${ENET_INCLUDE_DIR}")
find_library(ENET_LIBRARY enet)
message("-- enet library: ${ENET_LIBRARY}")
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(MTMultiplex PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

target_include_directories(MTMultiplex PRIVATE ${ENET_INCLUDE_DIR})
target_link_libraries(MTMultiplex PRIVATE ${ENET_LIBRARY})