cmake_minimum_required(VERSION 4.0)
project(HyComm VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(HC_BUILD_EXAMPLES "Build HyComm Examples" OFF)

include(FindModules)

add_subdirectory(src)

if (HC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()