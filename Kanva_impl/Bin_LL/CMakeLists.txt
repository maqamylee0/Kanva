cmake_minimum_required(VERSION 3.10)
project(Bin_LL)

set(CMAKE_CXX_STANDARD 14)

add_executable(Bin_LL main.cpp)
target_link_libraries(Bin_LL -pthread)
