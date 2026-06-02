cmake_minimum_required(VERSION 3.20)
project(CoDriverEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# MSVC: force UTF-8 for Chinese comments in source
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Eigen3 - header-only, download source without running its CMake
# Note: conanfile.txt also declares eigen/3.4.0 for local dev (Conan).
# CI uses FetchContent (below); local dev can use `conan install .`
include(FetchContent)
FetchContent_Declare(
    Eigen3
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(Eigen3)
if(NOT Eigen3_POPULATED)
    FetchContent_Populate(Eigen3)
endif()
set(EIGEN3_INCLUDE_DIR ${eigen3_SOURCE_DIR})

# Library
add_library(codriver_engine STATIC
    src/kalman_filter.cpp
    src/corner_detector.cpp
    src/root_cause.cpp
    src/coach_template.cpp
    src/coord_transform.cpp
    src/lap_timer.cpp
    src/brake_detector.cpp
    src/corner_speed_compare.cpp
    src/c_api.cpp
)

target_include_directories(codriver_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EIGEN3_INCLUDE_DIR}
)

# Tests
enable_testing()
add_executable(test_engine
    tests/test_main.cpp
)
target_include_directories(test_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EIGEN3_INCLUDE_DIR}
)
target_link_libraries(test_engine PRIVATE codriver_engine)
add_test(NAME EngineTests COMMAND test_engine)
