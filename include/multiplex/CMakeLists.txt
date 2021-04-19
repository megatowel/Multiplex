set (INCLUDE_FILES "base.hpp" "client.hpp" "server.hpp" "external.h" "packing.hpp" "multiplex.hpp")
source_group (include FILES ${MULTIPLEX_INCLUDE})
target_sources (multiplex PRIVATE
    ${INCLUDE_FILES}
)
target_include_directories(multiplex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)