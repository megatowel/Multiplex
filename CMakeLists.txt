# CMakeLists.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)
project ("MTMultiplex")

add_subdirectory ("./ENet")

set(MTMultiplexsrc "Base.cpp" "Client.cpp" "Server.cpp" "External.cpp" "External.h" "Packing.cpp" "Packing.h")

add_library(MTMultiplexlib OBJECT ${MTMultiplexsrc})

set_property(TARGET MTMultiplexlib PROPERTY POSITION_INDEPENDENT_CODE 1)

set_property(TARGET MTMultiplexlib PROPERTY CXX_STANDARD 20)

target_include_directories(MTMultiplexlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ENet/include/)
target_link_libraries(MTMultiplexlib PRIVATE enet)

add_library(MTMultiplex_shared SHARED $<TARGET_OBJECTS:MTMultiplexlib>)
add_library(MTMultiplex_static STATIC $<TARGET_OBJECTS:MTMultiplexlib>)

# Link WinSock2 if on Windows for ENet
if (WIN32)
	target_link_libraries(MTMultiplex_shared PRIVATE "ws2_32")
	target_link_libraries(MTMultiplex_static PRIVATE "ws2_32")
	if (MSVC)
		target_link_libraries(MTMultiplex_shared PRIVATE "Winmm")
		target_link_libraries(MTMultiplex_static PRIVATE "Winmm")
	endif()
elseif(UNIX)
endif()
