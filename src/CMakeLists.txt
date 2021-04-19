set (SOURCE_FILES "base.cpp" "client.cpp" "server.cpp" "external.cpp" "packing.cpp")
source_group (source FILES ${MULTIPLEX_SOURCE})
target_sources(multiplex PRIVATE 
    ${SOURCE_FILES}
)