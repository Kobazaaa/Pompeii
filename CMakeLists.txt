cmake_minimum_required(VERSION 3.27)

# Project Name
project(Pompeii)
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/project/src")

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/project")
