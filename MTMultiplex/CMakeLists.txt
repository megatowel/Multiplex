# CMakeLists.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)
project ("MTMultiplex")

add_subdirectory ("./ENet")

add_library(MTMultiplex SHARED "Base.cpp" "Client.cpp" "Server.cpp" "External.cpp" "External.h" "Packing.cpp" "Packing.h")
set_property(TARGET MTMultiplex PROPERTY CXX_STANDARD 20)

target_include_directories(MTMultiplex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../MTMultiplex/ENet/include/)
target_link_libraries(MTMultiplex PRIVATE enet)

# Link WinSock2 if on Windows for ENet
if (WIN32)
	target_link_libraries(MTMultiplex PRIVATE "Ws2_32")
	target_link_libraries(MTMultiplex PRIVATE "Winmm")
elseif(UNIX)
endif()

