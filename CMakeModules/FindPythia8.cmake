#
# (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
#
# See file AUTHORS for a list of contributors.
#
# This software is distributed under the terms of the GNU General Public
# Licence version 3 (GPL Version 3). See file LICENSE for a full version of
# the license.
#

#################################################
#
# run pythia8-config and interpret result
#

function (Pythia8_CONFIG_ option variable type doc)
  execute_process(COMMAND ${Pythia8_CONFIG} ${option}
    OUTPUT_VARIABLE _local_out_
    RESULT_VARIABLE _local_res_)
  string(REGEX REPLACE "\n$" "" _local_out_ "${_local_out_}")
  if (NOT ${_local_res_} EQUAL 0)
    message ("Error in running ${Pythia8_CONFIG} ${option}")
  else ()
    set (${variable} "${_local_out_}" CACHE ${type} ${doc})
  endif ()
endfunction (Pythia8_CONFIG_)
  


#################################################
# 
# Searched Pythia8 on system. Expect pythia8-config in PATH, or typical installation location
#
# This module defines
# Pythia8_INCLUDE_DIR   where to locate Pythia.h file
# Pythia8_LIBRARY       (not cached) the libraries to link against to use Pythia8
# Pythia8_VERSION 
#

set (SEARCH_Pythia8_
  ${PYTHIA8_DIR}
  $ENV{PYTHIA8_DIR}
  ${PYTHIA8}
  $ENV{PYTHIA8}
  ${PYTHIA8DIR}
  $ENV{PYTHIA8DIR}
  ${PYTHIA8_ROOT}
  $ENV{PYTHIA8_ROOT}
  /opt/pythia8)

find_program (Pythia8_CONFIG
  NAME pythia8-config
  PATHS ${SEARCH_Pythia8_}
  PATH_SUFFIXES "/bin"
  DOC "The location of the pythia8-config script")

if (Pythia8_CONFIG)
  set (HAVE_Pythia8 1 CACHE BOOL "presence of pythia8, found via pythia8-config")
  Pythia8_CONFIG_ ("--includedir" Pythia8_INCLUDE_DIR PATH "pythia8 include directory")
  Pythia8_CONFIG_ ("--prefix" Pythia8_PREFIX PATH "pythia8 prefix directory")
  find_library (Pythia8_LIBRARY NAMES "libpythia8.a" "libpythia8.so" PATH_SUFFIXES "lib" PATHS ${Pythia8_PREFIX} NO_DEFAULT_PATH DOC "pythia8 library")
  set (Pythia8_VERSION "n/a")
else ()
  set (HAVE_Pythia8 1 CACHE BOOL "presence of pythia8, found via include/lib")

  # if we get here, pythia8-config was not found by CMake so we use
  # CMake to try and find Pythia8 for us (but let the user know first).
  # We set these variables to exactly match the format of pythia8-config.
  # If any one of the variables is not found, CMake will automatically report
  # that Pythia8 is NOT FOUND (which is what we want).
  message ("pythia8-config was not found. Attempting to manually locate Pythia8...")

  # find the main header
  find_path (Pythia8_INCLUDE_DIR NAME "Pythia8/Pythia.h" PATH_SUFFIXES "include" PATHS ${SEARCH_Pythia8_})

  # and find the main library
  find_library (Pythia8_LIBRARY NAMES "libpythia8.a" "libpythia8.so" PATH_SUFFIXES "lib" PATHS ${SEARCH_Pythia8_})
endif ()

# also determine Pythia8 detailed version number
if (EXISTS "${Pythia8_INCLUDE_DIR}/Pythia8/Pythia.h")
  file (READ "${Pythia8_INCLUDE_DIR}/Pythia8/Pythia.h" PYTHIA_H_DATA)
  string (REGEX MATCH "#define PYTHIA_VERSION_INTEGER ([0-9]*)" test "${PYTHIA_H_DATA}")
  set (Pythia8_VERSION ${CMAKE_MATCH_1})
endif ()

# standard cmake infrastructure:
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Pythia8
  "Did not find system-level Pythia8. Switching to CORSIKA version."
  Pythia8_INCLUDE_DIR Pythia8_LIBRARY Pythia8_VERSION)
mark_as_advanced (Pythia8_INCLUDE_DIR Pythia8_LIBRARY Pythia8_VERSION)
