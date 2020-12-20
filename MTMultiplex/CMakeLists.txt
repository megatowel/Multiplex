# CMakeList.txt : CMake project for MTMultiplex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)
add_subdirectory ("./ENet")
# Add source to this project's executable.
add_library(MTMultiplex SHARED "Base.cpp" "Client.cpp" "Server.cpp" "External.cpp" "External.h" "Packing.cpp" "Packing.h")
set_property(TARGET MTMultiplex PROPERTY CXX_STANDARD 14)

target_include_directories(MTMultiplex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../MTMultiplex/ENet/include/)
target_link_libraries(MTMultiplex PRIVATE enet)

# Copy the DLL to the test folder if on Windows
if (WIN32)
	add_custom_command(TARGET MTMultiplex POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_BINARY_DIR}/MTMultiplex/MTMultiplex.dll" "${PROJECT_BINARY_DIR}/MTMultiplex-Test/")
endif()

# Link WinSock2 if on Windows for ENet
if (WIN32)
	target_link_libraries(MTMultiplex PRIVATE "Ws2_32.lib")
	target_link_libraries(MTMultiplex PRIVATE "Winmm.lib")
elseif(UNIX)
endif()

