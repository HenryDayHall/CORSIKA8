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

function (_Pythia8_CONFIG_ option variable type doc)
  execute_process(COMMAND ${Pythia8_CONFIG} ${option}
    OUTPUT_VARIABLE _local_out_
    RESULT_VARIABLE _local_res_)
  string(REGEX REPLACE "\n$" "" _local_out_ "${_local_out_}")
  if (NOT ${_local_res_} EQUAL 0)
    message ("Error in running ${Pythia8_CONFIG} ${option}")
  else ()
    set (${variable} "${_local_out_}" CACHE ${type} ${doc})
  endif ()
endfunction (_Pythia8_CONFIG_)
  


#################################################
# 
# Searched Pythia8 on system. Expect pythia8-config in PATH, or typical installation location
#
# This module defines
# Pythia8_INCLUDE_DIR   where to locate Pythia.h file
# Pythia8_LIBRARIES     (not cached) the libraries to link against to use Pythia8
#

set (_SEARCH_Pythia8_
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
  PATHS ${_SEARCH_PYTHIA8_}
  PATH_SUFFIXES "/bin"
  DOC "The location of the pythia8-config script")

if (Pythia8_CONFIG)
  set (HAVE_Pythia8 1 CACHE BOOL "presence of pythia8, found via pythia8-config")

  _Pythia8_CONFIG_ ("--includedir" Pythia8_INCLUDE_DIR PATH "pythia8 include directory")
  _Pythia8_CONFIG_ ("--libdir" Pythia8_LIBRARY_DIR PATH "pythia8 lib directory")
else()

  # if we get here, pythia8-config was not found by CMake so we use
  # CMake to try and find Pythia8 for us (but let the user know first).
  # We set these variables to exactly match the format of pythia8-config.
  # If any one of the variables is not found, CMake will automatically report
  # that Pythia8 is NOT FOUND (which is what we want).
  message(WARNING
    "pythia8-config was not found. Attempting to manually locate Pythia8...")

  # find the main header
  find_path(Pythia8_INCLUDE_DIR "Pythia8/Pythia.h")

  # and find the main library
  find_library(Pythia8_LIBRARY_DIR "libpythia8.so")

  # and set our best guess
  message("Found Pythia.h in ${Pythia8_INCLUDE_DIR}")
  message("Found libpythia8.so at ${Pythia8_LIBRARY_DIR}")
endif ()

# standard cmake infrastructure:
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Pythia8 DEFAULT_MSG Pythia8_CONFIG)
mark_as_advanced (Pythia8_INCLUDE_DIR Pythia8_LIBRARY_DIR)
