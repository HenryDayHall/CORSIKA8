cmake_minimum_required (VERSION 3.4.3)

project (corsika VERSION 8.0.0 DESCRIPTION "CORSIKA C++ project")

# ignore many irrelevant Up-to-date messages during install
set (CMAKE_INSTALL_MESSAGE LAZY)

# --std=c++14
set (CMAKE_CXX_STANDARD 14)


#add_custom_target (corsika_pre_build)
#add_custom_command (TARGET corsika_pre_build PRE_BUILD COMMAND "${PROJECT_SOURCE_DIR}/pre_compile.py")

# dependencies
find_package (Boost 1.40 COMPONENTS program_options REQUIRED)
find_package (Eigen3 3.3 REQUIRED)
find_package (HDF5)

# order of subdirectories 
add_subdirectory (ThirdParty)
add_subdirectory (Documentation)
add_subdirectory (Framework)
#add_subdirectory (Processes)
add_subdirectory (Main)

