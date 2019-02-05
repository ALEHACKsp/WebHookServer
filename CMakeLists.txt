cmake_minimum_required(VERSION 3.2)
project(WebHook)

set(CMAKE_CXX_STANDARD 14)

set(
    SOURCES
    source/main.cpp
    source/server.cpp
    source/client.cpp
    source/common.cpp
)

add_executable(WebHook ${SOURCES})
include_directories(include/)

# For linux.
target_link_libraries(WebHook pthread)