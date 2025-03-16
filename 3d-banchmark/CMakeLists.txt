cmake_minimum_required(VERSION 3.16)

project(3d-banchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    3d-banchmark
    main.cpp
)
