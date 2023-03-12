cmake_minimum_required(VERSION 3.24)
project(scop DESCRIPTION "a small 3D object loader")
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	message(STATUS "Enabling Debug mode")
	add_definitions(-DDEBUG=true)
else ()
	message(STATUS "Disabling Debug mode")
	add_definitions(-DDEBUG=false)
endif ()

set(SRC_PARSER
		include/parser/parser.h src/parser/parser.cpp
		include/parser/line_objects.h
		src/parser/line_parser.cpp
		src/parser/exceptions.cpp
		include/parser/utils.h src/parser/utils.cpp)

set(SRC_GRAPHICS
		include/graphics/vulkan.h src/graphics/vulkan.cpp
		include/graphics/utils.h src/graphics/utils.cpp
		include/graphics/debug.h src/graphics/debug.cpp
		include/graphics/renderer.h src/graphics/renderer.cpp
		include/graphics/queue_families.h src/graphics/queue_families.cpp
		include/graphics/swap_chain.h src/graphics/swap_chain.cpp)

set(SRC_MAIN
		src/main.cpp
		include/application.h src/application.cpp)

add_executable(scop ${SRC_MAIN} ${SRC_PARSER} ${SRC_GRAPHICS})

# Setup GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw-3.3.8)

target_include_directories(scop
		PRIVATE include
		PUBLIC ${Vulkan_INCLUDE_DIRS})

target_link_libraries(scop
		PRIVATE ${Vulkan_LIBRARIES}
		PRIVATE glfw)
target_link_libraries(scop PRIVATE debug $ENV{VULKAN_SDK}/Lib/shaderc_combinedd.lib)
target_link_libraries(scop PRIVATE optimized $ENV{VULKAN_SDK}/Lib/shaderc_combined.lib)