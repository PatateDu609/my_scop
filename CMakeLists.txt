project(scop DESCRIPTION "a small 3D object loader")

cmake_minimum_required(VERSION 3.16)

include_directories(include)

set(CMAKE_CXX_STANDARD 20)

add_executable(scop
		src/main.cpp
		include/parser/parser.h src/parser/parser.cpp include/parser/line_objects.h src/parser/line_parser.cpp src/parser/exceptions.cpp include/parser/utils.h src/parser/utils.cpp include/application.h include/application.h src/application.cpp)