cmake_minimum_required(VERSION 3.16)

project(3d-banchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    3d-banchmark
    main.cpp
)
target_compile_options(3d-banchmark PRIVATE /O2 /EHsc)

add_executable(
    3d-banchmark-openmp
    openmp_approach.cpp
)
 #cl /O2 /EHsc /openmp:llvm openmp_rewrite.cpp
target_compile_options(3d-banchmark-openmp PRIVATE /O2 /EHsc /openmp:llvm)
