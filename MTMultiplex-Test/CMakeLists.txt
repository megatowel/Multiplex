# CMakeLists.txt : CMake project for MTMultiplexTest, include source and define
# project specific logic here.
#

# We're building an executable.
add_executable (MTMultiplexTest MTMultiplexTest.cpp MTMultiplexTest.h)

target_include_directories (MTMultiplexTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../MTMultiplex/)
target_link_libraries (MTMultiplexTest PRIVATE MTMultiplex)

if (UNIX)
	find_package (Threads)
	target_link_libraries (MTMultiplexTest PRIVATE ${CMAKE_THREAD_LIBS_INIT})
endif()