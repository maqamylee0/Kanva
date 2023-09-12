cmake_minimum_required(VERSION 3.0)
project(Kanva)

set(CMAKE_CXX_STANDARD 17)
include_directories(common Kanva_impl Kanva_RS)
include_directories(common_TB
        common_TB/recordmgr
        common_TB/papi
        common_TB/kcas
        common_TB/descriptors
        common_TB/dcss
        common_TB/atomic_ops
        common_TB/rlu
        common_TB/rq
        common_TB/urcu)
add_executable(Kanva_benchmark Kanva_benchmark.cpp)
target_link_libraries(Kanva_benchmark pthread)
add_executable(Kanva_ycsb Kanva_benchmark_ycsb.cpp)
target_link_libraries(Kanva_ycsb pthread)
add_executable(Finedex_benchmark finedex_benchmark.cpp)
target_link_libraries(Finedex_benchmark pthread)
add_executable(Finedex_YCSB Finedex_benchmark_YCSB.cpp)
target_link_libraries(Finedex_YCSB pthread)
add_executable(ABTree_Benchmark AB_Tree_benchmark.cpp)
target_link_libraries(ABTree_Benchmark pthread)
add_executable(ABTree_YCSB ABTree_YCSB.cpp)
target_link_libraries(ABTree_YCSB pthread)
add_executable(CIST_benchmark CIST_benchmark.cpp)
target_link_libraries(CIST_benchmark pthread)
add_executable(CIST_YCSB CIST_YCSB.cpp)
target_link_libraries(CIST_YCSB pthread)
add_executable(ElimABtree_benchmark ElimABTree_benchmark.cpp)
target_link_libraries(ElimABtree_benchmark pthread)
add_executable(ElimABtree_YCSB ElimABtree_YCSB.cpp)
target_link_libraries(ElimABtree_YCSB pthread)
add_executable(Kanva_benchmark_RS_MR Kanva_benchmark_RS_MR.cpp)
target_link_libraries(Kanva_benchmark_RS_MR pthread)
