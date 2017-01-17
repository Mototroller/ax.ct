cmake_minimum_required (VERSION 2.8.7)
project (AX_CT_TEST)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")

set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

set(SRC_LIST tests/tests.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ax.core/include)
include_directories(${PROJECT_NAME} include)

add_executable(${PROJECT_NAME} ${SRC_LIST})
