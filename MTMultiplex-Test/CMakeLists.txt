# CMakeLists.txt : CMake project for MTMultiplexTest, include source and define
# project specific logic here.
#

# We're building an executable.
add_executable (MTMultiplexTest MTMultiplexTest.cpp MTMultiplexTest.h)
set_property(TARGET MTMultiplexTest PROPERTY CXX_STANDARD 20)

target_include_directories (MTMultiplexTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../MTMultiplex/)
target_link_libraries (MTMultiplexTest PRIVATE MTMultiplex)

if (UNIX)
	if (MINGW)
		option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
		add_subdirectory(mingw_stdthreads)
		target_link_libraries(MTMultiplexTest PRIVATE mingw_stdthreads)
	else()
		find_package (Threads)
		target_link_libraries (MTMultiplexTest PRIVATE ${CMAKE_THREAD_LIBS_INIT})
	endif()
endif()