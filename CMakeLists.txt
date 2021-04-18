cmake_minimum_required (VERSION 3.18)

project (multiplex)
option (MULTIPLEX_BUILD_STATIC "Build Multiplex for static linking instead of dynamic linking")

# Build and include ENet
add_subdirectory ("./ENet")

# Source files to build
set (MULTIPLEX_SOURCE "Base.cpp" "Client.cpp" "Server.cpp" "External.cpp" "Packing.cpp")
source_group (source FILES ${MULTIPLEX_SOURCE})

add_library (Multiplex OBJECT ${MULTIPLEX_SOURCE})

set_property (TARGET Multiplex PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property (TARGET Multiplex PROPERTY CXX_STANDARD 20)

target_include_directories (Multiplex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ENet/include/)

IF (${MULTIPLEX_BUILD_STATIC})
	add_library (libmultiplex STATIC $<TARGET_OBJECTS:Multiplex>)
ELSE ()
	add_library (libmultiplex SHARED $<TARGET_OBJECTS:Multiplex>)
ENDIF ()

target_link_libraries (libmultiplex PRIVATE enet)

set_property (TARGET libmultiplex PROPERTY OUTPUT_NAME multiplex)

# Link WinSock2 if on Windows (for ENet)
IF (WIN32)
	target_link_libraries(libmultiplex PRIVATE "ws2_32")
	IF (MSVC)
		target_link_libraries(libmultiplex PRIVATE "Winmm")
	ENDIF ()
ELSEIF (UNIX)
ENDIF ()
