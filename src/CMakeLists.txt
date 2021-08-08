set (SOURCE_FILES "base.cpp" "server.cpp" "packing.cpp" "types.cpp" "client.cpp")
source_group (source FILES ${MULTIPLEX_SOURCE})
target_sources(multiplex PRIVATE 
    ${SOURCE_FILES}
)