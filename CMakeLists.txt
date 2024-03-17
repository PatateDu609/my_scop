cmake_minimum_required(VERSION 3.24)
project(scop DESCRIPTION "a small 3D object loader")

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(
        Doxygen
        REQUIRED dot
        OPTIONAL_COMPONENTS mscgen
)

include(FetchContent)

# Setup Glfw
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # tag/3.4
)
FetchContent_GetProperties(glfw)
if (NOT glfw_POPULATED)
    FetchContent_Populate(glfw)

    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif ()

# Setup shaderc
FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc
        GIT_TAG d792558a8902cb39b1c237243cc4edab226513a5 # tag/v2024.0
)
FetchContent_GetProperties(shaderc)
if (NOT shaderc_POPULATED)
    FetchContent_Populate(shaderc)
    set($ENV{GIT_SYNC_DEPS_QUIET} 1)
    execute_process(
            COMMAND python3 utils/git-sync-deps
            WORKING_DIRECTORY ${shaderc_SOURCE_DIR}
            COMMAND_ERROR_IS_FATAL ANY
            OUTPUT_FILE git-sync-deps.out
    )

    set(SHADERC_SKIP_TESTS ON CACHE INTERNAL "Disables tests in ShaderC")
    set(SHADERC_SKIP_EXAMPLES ON CACHE INTERNAL "Disables examples in ShaderC")
    set(SHADERC_SKIP_INSTALL ON CACHE INTERNAL "Disables installation in ShaderC")

    add_subdirectory(${shaderc_SOURCE_DIR} ${shaderc_BINARY_DIR})
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Enabling Debug mode")
    set(SCOP_DEBUG ON)
else ()
    message(STATUS "Disabling Debug mode")
    set(SCOP_DEBUG OFF)
endif ()

set(SRC_PARSER
        include/parser/parser.h src/parser/parser.cpp
        include/parser/line_objects.h src/parser/line_parser.cpp
        src/parser/exceptions.cpp
        include/parser/utils.h src/parser/utils.cpp)

set(SRC_GRAPHICS
        include/graphics/vulkan.h src/graphics/vulkan.cpp
        include/graphics/utils.h src/graphics/utils.cpp
        include/graphics/debug.h src/graphics/debug.cpp
        include/graphics/renderer.h src/graphics/renderer.cpp
        include/graphics/queue_families.h src/graphics/queue_families.cpp
        include/graphics/swap_chain.h src/graphics/swap_chain.cpp
        include/graphics/shaders.h src/graphics/shaders.cpp
        src/graphics/pipeline.cpp include/graphics/pipeline.h)

set(SRC_MAIN
        src/main.cpp
        include/application.h src/application.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_MAIN} ${SRC_PARSER} ${SRC_GRAPHICS})
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -Wall -Wextra)

if (${SCOP_DEBUG})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEBUG=true)
else ()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEBUG=false)
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -Werror)
endif ()

target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE include
        PUBLIC ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${CMAKE_PROJECT_NAME}
        PRIVATE ${Vulkan_LIBRARIES}
        PRIVATE glfw)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE shaderc)

set(DOXYGEN_OUTPUT_DIRECTORY docs)
set(DOXYGEN_CREATE_SUBDIRS YES)
set(DOXYGEN_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

set(DOXYGEN_CALLER_GRAPH YES)
set(DOXYGEN_CALL_GRAPH YES)
set(DOXYGEN_CLASS_GRAPH YES)
set(DOXYGEN_DIR_GRAPH_MAX_DEPTH 2)
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_TEMPLATE_RELATIONS YES)
set(DOXYGEN_INCLUDE_GRAPH YES)
set(DOXYGEN_INCLUDED_BY_GRAPH YES)

set(DOXYGEN_DOT_IMAGE_FORMAT svg)
set(DOXYGEN_INTERACTIVE_SVG YES)
set(DOXYGEN_DOT_MAX_NODES 200)
set(DOXYGEN_DOT_GRAPH_DEPTH 50)

set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_EXTRACT_LOCAL_CLASSES YES)
set(DOXYGEN_EXTRACT_ANON_NSPACES YES)
set(DOXYGEN_EXTRACT_PRIV_VIRTUAL YES)

set(DOXYGEN_GENERATE_TREEVIEW YES)

set(DOXYGEN_UML_LOOK YES)
set(DOXYGEN_UML_LIMIT_NUM_FIELDS 20)

if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(DOXYGEN_CASE_SENSE_NAMES NO)
endif ()

doxygen_add_docs(
        doxygen
        ${SRC_MAIN} ${SRC_PARSER} ${SRC_GRAPHICS}
)
