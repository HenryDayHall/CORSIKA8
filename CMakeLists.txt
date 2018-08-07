cmake_minimum_required (VERSION 3.4.3)

set (CMAKE_CXX_STANDARD 14)

project (corsika VERSION 8.0.0 DESCRIPTION "CORSIKA C++ project")

#add_custom_target (corsika_pre_build)
#add_custom_command (TARGET corsika_pre_build PRE_BUILD COMMAND "${PROJECT_SOURCE_DIR}/pre_compile.py")

find_package (Boost 1.40 COMPONENTS program_options REQUIRED)
find_package (Eigen3 3.3 REQUIRED)

add_subdirectory (Documentation)
add_subdirectory (Framework)
#add_subdirectory (Processes)
add_subdirectory (Main)

