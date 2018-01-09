cmake_minimum_required(VERSION 3.9)
project(IIAP_dtw)

set(CMAKE_CXX_STANDARD 11)

add_executable(IIAP_dtw Searching_multi.cpp)
add_executable(eval test.cpp)