cmake_minimum_required(VERSION 3.26)
project(k_means)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

include_directories("C:/Users/jaco3/OneDrive/Documenti/Unifi/Magistrale/ParallelProgramming/Elaborati/k-means/libraries")

add_executable(k_means_sequential_AoS k-means_sequential_AoS.cpp libraries/INIReader.cpp libraries/ini.c)
add_executable(k_means_sequential_SoA k-means_sequential_SoA.cpp libraries/INIReader.cpp libraries/ini.c)
add_executable(k_means_parallel k-means_parallel.cpp libraries/INIReader.cpp libraries/ini.c)

target_link_libraries(k_means_sequential_AoS)
target_link_libraries(k_means_sequential_SoA)
target_link_libraries(k_means_parallel)