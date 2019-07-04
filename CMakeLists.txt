cmake_minimum_required (VERSION 3.9)

# prevent in-source builds and give warning message
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}") 
  message (FATAL_ERROR "In-source builds are disabled.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake will now create CMakeCache.txt and CMakeFiles/*.
          You must delete them, or cmake will refuse to work.")
endif () 

project (
  corsika
  VERSION 8.0.0
  DESCRIPTION "CORSIKA C++ project"
  LANGUAGES CXX
  )

# as long as there still are modules using it:
enable_language (Fortran)

# TEMPORARY: this should be removed, the sanitizers should be always enabled
option (WITH_CORSIKA_SANITIZERS_ENABLED "temporary way to globally disable sanitizers until the currently failing tests are fixed" OFF)
option (WITH_PYTHIA "flag to switch on/off pythia support" OFF)
option (WITH_COAST "flag to switch on/off COAST (reverse) interface" OFF)

# ignore many irrelevant Up-to-date messages during install
set (CMAKE_INSTALL_MESSAGE LAZY)

# directory for local cmake modules
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include (CorsikaUtilities) # a few cmake function

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS OFF)
enable_testing ()
set (CTEST_OUTPUT_ON_FAILURE 1)

# Set the possible values of build type for cmake-gui and command line check
set (ALLOWED_BUILD_TYPES Debug Release MinSizeRel RelWithDebInfo Coverage)
set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${ALLOWED_BUILD_TYPES})

# Set a default build type if none was specified
# by default: "Debug", if local ".git" directory is found, otherwise "Release"
set (DEFAULT_BUILD_TYPE "Release")
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set (DEFAULT_BUILD_TYPE "Debug")
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message (STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as no other was specified.")
  set (CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
      STRING "Choose the type of build." FORCE)
else (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  # Ignore capitalization when build type is selected manually and check for valid setting
  string (TOLOWER ${CMAKE_BUILD_TYPE} SELECTED_LOWER)
  string (TOLOWER "${ALLOWED_BUILD_TYPES}" BUILD_TYPES_LOWER)
  if (NOT SELECTED_LOWER IN_LIST BUILD_TYPES_LOWER)
    message (FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE} [allowed: ${ALLOWED_BUILD_TYPES}]")
  endif ()
  message (STATUS "Build type is: ${CMAKE_BUILD_TYPE}")
endif (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

# enable warnings and disallow non-standard language
# configure the various build types here, too
# FYI: optimizer flags: -O2 would not trade speed for size, neither O2/3 use fast-math
# debug: O0, relwithdebinfo: 02, release: O3, minsizerel: Os (all defaults)
set (CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra -Wno-ignored-qualifiers")
set (CMAKE_Fortran_FLAGS "-std=legacy")

# setup coverage build type
set (CMAKE_CXX_FLAGS_COVERAGE "-g --coverage")
set (CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
set (CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage")

# clang produces a lot of unecessary warnings without this:
add_compile_options ("$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-nonportable-include-path>")

# COAST - interface, this requires CORSIKA7 to be installed first
# COAST will allow you to program code in CORSIKA8 and execute it inside CORSIKA7
if (WITH_COAST)
  message (STATUS "Compiling CORSIKA8 for the use with COAST/corsika7.")
  add_compile_options ("-fPIC")
endif ()

# targets and settings needed to generate coverage reports
if (CMAKE_BUILD_TYPE STREQUAL Coverage)
  find_package (Perl REQUIRED)

  set (GCOV gcov CACHE STRING "gcov executable" FORCE)
  set (LCOV_BIN_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/lcov/bin")
  # collect coverage data
  add_custom_command (
    OUTPUT raw-coverage.info
    COMMAND ${CMAKE_COMMAND} -E echo "Note: you need to run ctest at least once to generate the coverage data"
    COMMAND ${LCOV_BIN_DIR}/lcov --gcov-tool=${GCOV} --directory . --capture --output-file raw-coverage.info
    )
  # remove uninteresting entries
  add_custom_command (
    OUTPUT coverage.info
    COMMAND ${LCOV_BIN_DIR}/lcov -q --remove raw-coverage.info "*/usr/*" --output-file coverage2.info
    COMMAND ${LCOV_BIN_DIR}/lcov --remove coverage2.info "*/ThirdParty/*" --output-file coverage.info
    COMMAND ${CMAKE_COMMAND} -E remove coverage2.info
    DEPENDS raw-coverage.info
    )
  # generate html report
  add_custom_command (
    OUTPUT coverage-report
    COMMAND ${LCOV_BIN_DIR}/genhtml coverage.info -o coverage-report
    DEPENDS coverage.info
    )
  add_custom_target (coverage DEPENDS coverage-report)
endif ()

# add call to ./do-copyright.py to run as unit-test-case
add_test (NAME copyright_notices COMMAND ./do-copyright.py WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# add call to do-clang-format.py checking to run as unit-test-case
add_test (NAME clang_format COMMAND ./do-clang-format.py WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if (WITH_PYTHIA)
  find_package (Pythia8) # optional
endif (WITH_PYTHIA)

# include potential ThirdParty code provided with CORSIKA
add_subdirectory (ThirdParty)

# check for Eigen3: either use ThirdParty/eigen3 or system-level installation
if (WITH_EIGEN3)
  string (TOLOWER ${WITH_EIGEN3} WITH_EIGEN3_LOWER)
  if (WITH_EIGEN3_LOWER EQUAL "system")
    find_package (Eigen3 REQUIRED)
  else ()
    list (APPEND CMAKE_MODULE_PATH "${WITH_EIGEN3}/cmake")
    set (EIGEN3_INCLUDE_DIR "${WITH_EIGEN3}" CACHE PATH "eigen3 directory") 
    find_package (Eigen3 REQUIRED)
  endif ()
else (WITH_EIGEN3)
  list (APPEND CMAKE_MODULE_PATH "${LOCAL_Eigen3_VERSION}/cmake")
  set (EIGEN3_INCLUDE_DIR "${LOCAL_Eigen3_VERSION}" CACHE PATH "eigen3 directory")
  find_package (Eigen3 REQUIRED)
endif (WITH_EIGEN3)

# some final info output
message ("Use eigen3 version: ${EIGEN3_VERSION} from ${EIGEN3_INCLUDE_DIR}")


# order of subdirectories
add_subdirectory (Framework)
add_subdirectory (Environment)
add_subdirectory (Stack)
add_subdirectory (Setup)
add_subdirectory (Processes)
add_subdirectory (Documentation)
add_subdirectory (Main)
add_subdirectory (Tools)
if (WITH_COAST)
  add_subdirectory (COAST)
endif ()


# final summary output
include (FeatureSummary)
feature_summary (WHAT ALL)
