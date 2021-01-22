cmake_minimum_required (VERSION 3.9)

#+++++++++++++++++++++++++++++
# project name
#
set (c8_version 8.0.0)
project (
  corsika
  VERSION ${c8_version}
  DESCRIPTION "CORSIKA C++ project (alpha status)"
  LANGUAGES CXX
  )

#+++++++++++++++++++++++++++++
# prevent in-source builds and give warning message
#
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}") 
  message (FATAL_ERROR "In-source builds are disabled.
    Please create a build-dir and use `cmake <source-dir>` inside it.
    NOTE: cmake will now create CMakeCache.txt and CMakeFiles/*.
          You must delete them, or cmake will refuse to work.")
endif ()

#++++++++++++++++++++++++++++
# cmake version-specific settings
#
# https://cmake.org/cmake/help/latest/policy/CMP0079.html
if (${CMAKE_VERSION} VERSION_GREATER "3.13.0")
  cmake_policy (SET CMP0079 NEW)
endif ()
if (POLICY CMP0079)
    cmake_policy (SET CMP0079 NEW)
endif ()

#+++++++++++++++++++++++++++++
# as long as there still are modules using it:
#
enable_language (Fortran)
set (CMAKE_Fortran_FLAGS "-std=legacy -Wfunction-elimination")

#+++++++++++++++++++++++++++++
# warn user if system is not UNIX
#
if (NOT UNIX)
  message (FATAL_ERROR "| CORSIKA8 > This is an unsupported system.")
endif ()

#+++++++++++++++++++++++++++++
# cmake path dir, and cmake config
#
set (CORSIKA8_CMAKE_DIR "${PROJECT_SOURCE_DIR}/cmake")
set (CMAKE_MODULE_PATH  "${CORSIKA8_CMAKE_DIR}" ${CMAKE_MODULE_PATH})
include (CorsikaUtilities) # extra cmake function
set (CMAKE_VERBOSE_MAKEFILE  OFF) # this can be done with `make VERBOSE=1`
# ignore many irrelevant Up-to-date messages during install
set (CMAKE_INSTALL_MESSAGE LAZY)

#+++++++++++++++++++++++++++++
# Setup hardware and infrastructure dependent defines
#
include (CorsikaDefines)

#+++++++++++++++++++++++++++++
# check if compiler is C++17 compliant
#
include (CheckCXXCompilerFlag)
check_CXX_compiler_flag ("--std=c++17" COMPILER_SUPPORTS_CXX17)
if (NOT COMPILER_SUPPORTS_CXX17)
 message (FATAL "| CORSIKA8 > The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif ()
# set CXX compile flags and options and warning settings
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS OFF)

#+++++++++++++++++++++++++++++
# Compiler and linker flags, settings
#
# enable warnings and disallow non-standard language
# configure the various build types here, too
# FYI: optimizer flags: -O2 would not trade speed for size, neither O2/3 use fast-math
# debug: O0, relwithdebinfo: 02, release: O3, minsizerel: Os (all defaults)
set (CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra -Wno-ignored-qualifiers")
set (DEFAULT_BUILD_TYPE "Release")

# clang produces a lot of unecessary warnings without this:
add_compile_options (
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-nonportable-include-path>
  )

#+++++++++++++++++++++++++++++
# Build types settings
#
# setup coverage build type
set (CMAKE_CXX_FLAGS_COVERAGE "-g --coverage")
set (CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
set (CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage")
# set a flag to inform code that we are in debug mode
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

#+++++++++++++++++++++++++++++
# Build type selection
#
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

#+++++++++++++++++++++++++++++
# Setup external dependencies.
#
include (conan) # self-provided in 'cmake' directory
#
# download and build all dependencies
conan_cmake_run (CONANFILE conanfile.txt
                 BASIC_SETUP CMAKE_TARGETS
                 BUILD missing
                 BUILD_TYPE "Release")
#
# add cnpy temporarily. As long as SaveBoostHistogram does not save to parquet directly
#
add_subdirectory (externals/cnpy)

#+++++++++++++++++++++++++++++
# Coverage
#
# targets and settings needed to generate coverage reports
if (CMAKE_BUILD_TYPE STREQUAL Coverage)
  find_package (Perl REQUIRED)

  # compile coverage under -O0 to avoid any optimization, function elimation etc.
  add_compile_options ("-O0")
  
  set (GCOV gcov CACHE STRING "gcov executable" FORCE)
  set (LCOV_BIN_DIR "${PROJECT_SOURCE_DIR}/externals/lcov/bin")
  # collect coverage data
  add_custom_command (
    OUTPUT raw-coverage.info
    COMMAND ${CMAKE_COMMAND} -E echo "Note: you need to run ctest at least once to generate the coverage data"
    COMMAND ${LCOV_BIN_DIR}/lcov --gcov-tool=${GCOV} --directory . --capture --output-file raw-coverage.info
    )
  # remove uninteresting entries
  add_custom_command (
    OUTPUT coverage.info
    COMMAND ${LCOV_BIN_DIR}/lcov -q --remove raw-coverage.info "*/usr/*" "/usr/*" --output-file coverage2.info
    COMMAND ${LCOV_BIN_DIR}/lcov --remove coverage2.info "*/externals/*" "*/tests/*" "*/sibyll2.3d.cpp" "*/.conan/*" "*/include/Pythia8/*" "${CMAKE_SOURCE_DIR}/modules/*" "${CMAKE_BINARY_DIR}/modules/*" --output-file coverage.info
    COMMAND ${CMAKE_COMMAND} -E remove coverage2.info
    DEPENDS raw-coverage.info
    )
  # generate html report
  add_custom_command (
    OUTPUT coverage-report
    COMMAND ${LCOV_BIN_DIR}/genhtml --demangle-cpp coverage.info -o coverage-report
    DEPENDS coverage.info
    )
  add_custom_target (coverage DEPENDS coverage-report)
endif ()

#+++++++++++++++++++++++++++++
# CTest config
#
enable_testing ()
if (${CMAKE_VERSION} VERSION_LESS "3.12.0")  
  list (APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
else (${CMAKE_VERSION} VERSION_LESS "3.12.0")
  set (CTEST_OUTPUT_ON_FAILURE 1) # this is for new versions of cmake
endif (${CMAKE_VERSION} VERSION_LESS "3.12.0")
set (CTEST_CUSTOM_COVERAGE_EXCLUDE "./tests/" "./examples/" "./modules/" "test*.(hpp/cpp)$")
# include this test only if NOT run on gitlab-ci; On CI this is a dedicated job:
if (NOT DEFINED ENV{CI})
  # add call to ./do-copyright.py to run as unit-test-case
  add_test (NAME copyright_notices COMMAND ./do-copyright.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif (NOT DEFINED ENV{CI})

#+++++++++++++++++++++++++++++
# Options
#
# HISTORY option selection
option (WITH_HISTORY "Flag to switch on/off HISTORY" ON)

#+++++++++++++++++++++++++++++
# Externals
#
set (Python_ADDITIONAL_VERSIONS 3)
find_package (PythonInterp 3 REQUIRED)

#+++++++++++++++++++++++++++++++
# exporting of CMake targets
#
include (CMakePackageConfigHelpers)

# list of targets that will be INSTALLed and EXPORTed
# Important: all those targets must be individually INSTALLed with "EXPORT CORSIKA8PublicTargets"
set (public_CORSIKA8_targets CORSIKA8)


#+++++++++++++++++++++++++++++
# CORSIKA8
#
add_library (CORSIKA8 INTERFACE)
set_target_properties (
  CORSIKA8
  PROPERTIES
  INTERFACE_CORSIKA8_MAJOR_VERSION 0
  COMPATIBLE_INTERFACE_STRING CORSIKA8_MAJOR_VERSION
  )
#  
target_include_directories (
  CORSIKA8
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
  ) 
# since CORSIKA8 is a header only library we must specify all link dependencies here:
target_link_libraries (
  CORSIKA8
  INTERFACE
  CONAN_PKG::eigen
  CONAN_PKG::spdlog
  CONAN_PKG::boost
  cnpy # for SaveBoostHistogram
  stdc++fs # experimental::filesystem
  )
# "src" is needed, since some headers (namely GeneratedParticleProperties.inc ec.) are produced by python script from e.g. ParticleData.xml
add_subdirectory (src)
add_subdirectory (documentation)

#+++++++++++++++++++++++++++++
#  = Add of subdirectories =
#

#+++++++++++++++++++++++++++++
# modules
#
set (CORSIKA_DATA_WITH_TEST ON) # we want to run the corsika-data unit test
add_subdirectory (modules/data) # this is corsika-data (submodule)
add_subdirectory (modules/pythia)
add_subdirectory (modules/sibyll)
add_subdirectory (modules/qgsjetII)
add_subdirectory (modules/urqmd)
add_subdirectory (modules/conex)
#
# we really need a better proposal CMakeList.txt file:
set (ADD_PYTHON OFF)
file (READ modules/CMakeLists_PROPOSAL.txt overwrite_CMakeLists_PROPOSAL_txt)
file (WRITE modules/proposal/CMakeLists.txt ${overwrite_CMakeLists_PROPOSAL_txt})
add_subdirectory (modules/proposal)
target_link_libraries (CORSIKA8 INTERFACE PROPOSAL)

#+++++++++++++++++++++++++++++++
# unit testing
# 
add_subdirectory (tests)

#+++++++++++++++++++++++++++++++
# installation
#
install (
  TARGETS CORSIKA8
  EXPORT CORSIKA8PublicTargets
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}
  )

#
# header only part
#
install (DIRECTORY corsika DESTINATION include)

configure_package_config_file (
  cmake/CORSIKA8Config.cmake.in
  ${PROJECT_BINARY_DIR}/CORSIKA8Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}
  )
  
write_basic_package_version_file (
  ${PROJECT_BINARY_DIR}/CORSIKA8ConfigVersion.cmake
  VERSION ${c8_version}
  COMPATIBILITY SameMajorVersion
  )

install (
  EXPORT CORSIKA8PublicTargets
  FILE CORSIKA8Targets.cmake
  NAMESPACE CORSIKA8::
  DESTINATION lib/cmake/CORSIKA8
  )

install (FILES
  ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake
  ${CMAKE_BINARY_DIR}/conaninfo.txt
  ${CMAKE_BINARY_DIR}/CORSIKA8Config.cmake
  ${CMAKE_BINARY_DIR}/CORSIKA8ConfigVersion.cmake
  DESTINATION lib/cmake/CORSIKA8
  )

#
# examples
#
install (DIRECTORY examples DESTINATION share/)

#
# tools
#
install (DIRECTORY tools DESTINATION share/)





#+++++++++++++++++++++++++++++++
#
# final summary output
#
include (FeatureSummary)
add_feature_info (HISTORY WITH_HISTORY "Full information on cascade history for particles.")
feature_summary (WHAT ALL)
