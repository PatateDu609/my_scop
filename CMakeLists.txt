project(scop DESCRIPTION "a small 3D object loader")

cmake_minimum_required(VERSION 3.16)

include_directories(include)

message("CXX Standard: ${CMAKE_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD 20)

add_executable(scop include/parser/parser.h src/main.cpp)