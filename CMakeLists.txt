cmake_minimum_required(VERSION 3.28)
project(HyComm VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(HC_BUILD_EXAMPLES "Build HyComm Examples" OFF)
option(HC_BUILD_TESTS "Build HyComm Tests" ON)

include(FindModules)

add_subdirectory(src)

if (HC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (HC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
