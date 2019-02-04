cmake_minimum_required(VERSION 3.2)
project(WebHook)

set(CMAKE_CXX_STANDARD 14)

set(
    SOURCES
    source/main.cpp
)

add_executable(WebHook ${SOURCES})
include_directories(include/)