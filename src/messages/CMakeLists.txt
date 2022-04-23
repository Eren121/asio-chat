find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ChatMessage.proto)

add_library(messages ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(messages PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(messages PUBLIC protobuf::libprotobuf)