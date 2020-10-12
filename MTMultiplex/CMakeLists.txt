# CMakeList.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Add source to this project's executable.
add_library(MTMultiplex SHARED "Base.cpp" "Client.cpp" "Server.cpp")
set_property(TARGET MTMultiplex PROPERTY CXX_STANDARD 14)

find_path(ENET_INCLUDE_DIR enet/enet.h)
message("-- enet include: ${ENET_INCLUDE_DIR}")
find_library(ENET_LIBRARY enet)
message("-- enet library: ${ENET_LIBRARY}")
find_package(nlohmann_json CONFIG REQUIRED)

if (WIN32)
	target_link_libraries(MTMultiplex PRIVATE "Ws2_32.lib")
	target_link_libraries(MTMultiplex PRIVATE "Winmm.lib")
elseif(UNIX)
	SET(CMAKE_CXX_FLAGS -pthread)
endif()

target_include_directories(MTMultiplex PRIVATE ${ENET_INCLUDE_DIR})
target_link_libraries(MTMultiplex PRIVATE ${ENET_LIBRARY})