cmake_minimum_required(VERSION 3.20)
project(Obligdatakom C)

set(CMAKE_C_STANDARD 11)

add_executable(HTTP_server HTTP_server.c)
target_link_libraries(HTTP_server ws2_32)