cmake_minimum_required (VERSION 3.9)
# we would need 3.16 to have CMP0097 for external subproject submodule (non) support

#+++++++++++++++++++++++++++++
# project name
#
project( CORSIKA8_Modules_Tests_Examples )
#
#
#+++++++++++++++++++++++++++++
# warn user if system is not UNIX
#
if(NOT UNIX)
  message(FATAL_ERROR "| CORSIKA8 > This is an unsupported system.")
endif()
#
#+++++++++++++++++++++++++++++
# cmake path dir
#
SET(CORSIKA8_CMAKE_DIR "${PROJECT_SOURCE_DIR}/cmake")
SET(CMAKE_MODULE_PATH  "${CORSIKA8_CMAKE_DIR}" ${CMAKE_MODULE_PATH})
SET(CMAKE_VERBOSE_MAKEFILE  ON)
#
#+++++++++++++++++++++++++++++
# check if compiler is C++17 compliant
#
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("--std=c++17" COMPILER_SUPPORTS_CXX17)
if(NOT COMPILER_SUPPORTS_CXX17)
 message(FATAL "| CORSIKA8 > The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif()

# set CXX compile flags and options and warning settings
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS OFF)

# enable warnings and disallow non-standard language
# configure the various build types here, too
# FYI: optimizer flags: -O2 would not trade speed for size, neither O2/3 use fast-math
# debug: O0, relwithdebinfo: 02, release: O3, minsizerel: Os (all defaults)
set (CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra -Wno-ignored-qualifiers")
set (CMAKE_Fortran_FLAGS "-std=legacy -Wfunction-elimination")

# clang produces a lot of unecessary warnings without this:
add_compile_options ("$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-nonportable-include-path>")

# ctest config
enable_testing ()
set (CTEST_OUTPUT_ON_FAILURE 1)


#+++++++++++++++++++++++++++++
# get CORSIKA8
#
find_package(CORSIKA8 REQUIRED)
include_directories(${CORSIKA8_INCLUDE_DIR})
# those are needed, since some headers (namely GeneratedParticleProperties.inc) are produced by python script from ParticleData.xml
add_subdirectory(corsika/framework) 
add_subdirectory(corsika/modules/sibyll) 
#
#+++++++++++++++++++++++++++++
# get Eigen3 
#
find_package( Eigen3 REQUIRED )
include_directories(${EIGEN3_INCLUDE_DIR})

#+++++++++++++++++++++++++++++
# get catch2 
#
find_package( Catch2 REQUIRED )

#+++++++++++++++++++++++++++++
# use spdlog
#
add_subdirectory(externals/spdlog)
add_dependencies(CORSIKA8 spdlog::spdlog)

#+++++++++++++++++++++++++++++
# get phys_units
#
find_package( PhysUnits REQUIRED )

#+++++++++++++++++++++++++++++
# get Pythia
#
find_package( Pythia8 REQUIRED )
if(Pythia_FOUND)
include_directories(${Pythia_INCLUDE_DIR})
endif(Pythia_FOUND)
#
#+++++++++++++++++++++++++++++
# get UrQMD
#
#find_package( UrQMD REQUIRED )
if(UrQMD_FOUND)
include_directories(${UrQMD_INCLUDE_DIR})
endif(UrQMD_FOUND)
#
#+++++++++++++++++++++++++++++
# get Sybill
#
# find_package( Sibyll REQUIRED )
add_subdirectory (dependencies/sibyll)
if(Sibyll_FOUND)
include_directories(${Sibyll_INCLUDE_DIR})
endif(Sibyll_FOUND)
#
#+++++++++++++++++++++++++++++
# get QGSJETII
#
#find_package( QGSJETII REQUIRED )
if(QGSJETII_FOUND)
include_directories(${QGSJETII_INCLUDE_DIR})
endif(QGSJETII_FOUND)
#
#
#+++++++++++++++++++++++++++++
# =~~~~~~~~~~~~~~~~~~~~~~~~~=
#  = Add of subdirectories =
# =~~~~~~~~~~~~~~~~~~~~~~~~~=
#+++++++++++++++++++++++++++++
#
# modules
# 
#add_subdirectory (dependencies/Pythia)
#add_subdirectory (dependencies/Sibyll)
#add_subdirectory (dependencies/QGSJetII)
#add_subdirectory (dependencies/UrQMD)
#
#+++++++++++++++++++++++++++++++
# tests
# 
add_subdirectory (tests/framework)
add_subdirectory (tests/media)
add_subdirectory (tests/stack)
add_subdirectory (tests/modules)
#
#+++++++++++++++++++++++++++++++
# examples
# 
#add_subdirectory (examples)
#
#+++++++++++++++++++++++++++++++
#
# final summary output
feature_summary (WHAT ALL)
