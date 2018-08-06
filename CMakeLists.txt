cmake_minimum_required (VERSION 3.4.3)

set (CMAKE_CXX_STANDARD 14)

project (CORSIKA VERSION 8.0.0.0 DESCRIPTION "CORSIKA C++ project")

find_package (Boost 1.40 COMPONENTS program_options REQUIRED)
find_package (Eigen3 3.3 REQUIRED)

add_subdirectory (Framework)
add_subdirectory (Main)
