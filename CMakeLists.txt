cmake_minimum_required(VERSION 3.17)
project(Asio)

set(CMAKE_CXX_STANDARD 17)

add_library(common common/session.cpp common/sized_session.cpp)
target_include_directories(common PUBLIC asio/asio/include)
target_include_directories(common PUBLIC common/)

add_executable(client src/client.cpp)
target_link_libraries(client common)

add_executable(server src/server.cpp)
target_link_libraries(server common)

# build automatically both
add_custom_target(both)
add_dependencies(both client server)

if(WIN32)
    target_compile_definitions(common PUBLIC _WIN32_WINNT=0x0601)
endif()