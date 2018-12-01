cmake_minimum_required (VERSION 3.4.3)

project (
  corsika
  VERSION 8.0.0
  DESCRIPTION "CORSIKA C++ project"
  LANGUAGES CXX
  )

# ignore many irrelevant Up-to-date messages during install
set (CMAKE_INSTALL_MESSAGE LAZY)

# directory for local cmake modules
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include (CorsikaUtilities) # a few cmake function

# enable warnings and disallow non-standard language
add_definitions(-Wall -pedantic -Wextra -Wno-ignored-qualifiers)

set (CMAKE_CXX_STANDARD 17)
enable_testing ()
set (CTEST_OUTPUT_ON_FAILURE 1)

ENABLE_LANGUAGE (Fortran)

# unit testing coverage, does not work yet
#include (CodeCoverage)
##set(COVERAGE_LCOV_EXCLUDES 'Documentation/*')
##setup_target_for_coverage(${PROJECT_NAME}_coverage ${PROJECT_TEST_NAME} coverage)
#SETUP_TARGET_FOR_COVERAGE_GCOVR_HTML (
#  NAME corsika_coverage
#  EXECUTABLE ctest
#  #-j ${PROCESSOR_COUNT}
#  #     DEPENDENCIES corsika
#  )

#add_custom_target (corsika_pre_build)
#add_custom_command (TARGET corsika_pre_build PRE_BUILD COMMAND "${PROJECT_SOURCE_DIR}/pre_compile.py")

# dependencies
find_package (Boost 1.60 REQUIRED)
find_package (Eigen3 REQUIRED)
#find_package (HDF5) # not yet needed

# order of subdirectories
add_subdirectory (ThirdParty)
#add_subdirectory (Utilities)
add_subdirectory (Framework)
add_subdirectory (Stack)
add_subdirectory (Setup)
add_subdirectory (Processes)
add_subdirectory (Documentation)
add_subdirectory (Main)
