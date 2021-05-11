cmake_minimum_required (VERSION 3.18...3.20)

project (multiplex)

# Options
option (MULTIPLEX_BUILD_STATIC "Build Multiplex for static linking instead of dynamic linking.")

IF (${MULTIPLEX_BUILD_STATIC})
	add_library (multiplex STATIC)
ELSE ()
	add_library (multiplex SHARED)
ENDIF ()

set_property (TARGET multiplex PROPERTY CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(include/multiplex)

add_subdirectory(lib/enet)
target_link_libraries (multiplex PRIVATE enet)
target_include_directories (multiplex PRIVATE lib/enet/include)

# Link WinSock2 if on Windows (for ENet)
IF (WIN32)
	target_link_libraries (multiplex PRIVATE "ws2_32")
	IF (MSVC)
		target_link_libraries (multiplex PRIVATE "Winmm")
	ENDIF ()
ENDIF ()

install (TARGETS multiplex
	LIBRARY DESTINATION lib
)
install (FILES ${MULTIPLEX_INCLUDE} 
	DESTINATION include/${OUTPUT_NAME}
)