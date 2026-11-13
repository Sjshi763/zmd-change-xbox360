cmake_minimum_required(VERSION 4.3)
project(atest)

set(CMAKE_CXX_STANDARD 26)

add_library(atest SHARED library.cpp)
target_link_libraries(atest PRIVATE xinput1_4)
