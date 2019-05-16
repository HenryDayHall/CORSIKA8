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

# ignore many irrelevant Up-to-date messages during install
set (CMAKE_INSTALL_MESSAGE LAZY)

# TEMPORARY: this should be removed, the sanitizers should be always enabled
option(CORSIKA_SANITIZERS_ENABLED "temporary way to globally disable sanitizers until the currently failing tests are fixed" OFF)

# directory for local cmake modules
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include (CorsikaUtilities) # a few cmake function

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS OFF)
enable_testing ()
set (CTEST_OUTPUT_ON_FAILURE 1)

# Set a default build type if none was specified
set (default_build_type "Release")
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set (default_build_type "Debug")
endif ()

# Set the possible values of build type for cmake-gui and command line check
set (allowed_build_types Debug Release MinSizeRel RelWithDebInfo Coverage)
set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${allowed_build_types})

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message (STATUS "Setting build type to '${default_build_type}' as no other was specified.")
  set (CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
else (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  # Ignore capitalization when build type is selected manually and check for valid setting
  string (TOLOWER ${CMAKE_BUILD_TYPE} selected_lower)
  string (TOLOWER "${allowed_build_types}" build_types_lower)
  if (NOT selected_lower IN_LIST build_types_lower)
    message (FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE} [allowed: ${allowed_build_types}]")
  endif ()
  message (STATUS "Build type is: ${CMAKE_BUILD_TYPE}")
endif (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

# enable warnings and disallow non-standard language
set (CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra -Wno-ignored-qualifiers")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g") # -O2 would not trade speed for size, neither O2/3 use fast-math
set (CMAKE_Fortran_FLAGS "-std=legacy")

# setup coverage target
set (CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
set (CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
set (CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage")

# clang produces a lot of unecessary warnings without this:
add_compile_options ("$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-nonportable-include-path>")

# COAST - interface
if (WITH_COAST)
  message (STATUS "Compiling CORSIKA8 for the use with COAST/corsika7.")
  add_compile_options ("-fPIC")
endif ()

# generate coverage report
if (CMAKE_BUILD_TYPE STREQUAL Coverage)
  find_package (Perl REQUIRED)
  set (GCOV gcov CACHE STRING "gcov executable" FORCE)
  set (LCOV_BIN_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/lcov/bin")
  # collect coverage data
  add_custom_command(OUTPUT raw-coverage.info
    COMMAND ${CMAKE_COMMAND} -E echo "Note: you need to run ctest at least once to generate the coverage data"
    COMMAND ${LCOV_BIN_DIR}/lcov --gcov-tool=${GCOV} --directory . --capture --output-file raw-coverage.info)
  # remove uninteresting entries
  add_custom_command(OUTPUT coverage.info
    COMMAND ${LCOV_BIN_DIR}/lcov --remove raw-coverage.info "*/usr/*" --output-file coverage2.info
    COMMAND ${LCOV_BIN_DIR}/lcov --remove coverage2.info "*/ThirdParty/*" --output-file coverage.info
    COMMAND ${CMAKE_COMMAND} -E remove coverage2.info
    DEPENDS raw-coverage.info)
  # generate html report
  add_custom_command(OUTPUT coverage-report
    COMMAND ${LCOV_BIN_DIR}/genhtml coverage.info -o coverage-report
    DEPENDS coverage.info)
  add_custom_target(coverage DEPENDS coverage-report)
endif ()

#add_custom_target (corsika_pre_build)
#add_custom_command (TARGET corsika_pre_build PRE_BUILD COMMAND "${PROJECT_SOURCE_DIR}/pre_compile.py")

find_package (Pythia8) # optional
  
find_package (Eigen3 REQUIRED)
#find_package (HDF5) # not yet needed

# order of subdirectories
add_subdirectory (ThirdParty)
#add_subdirectory (Utilities)
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
endif()
