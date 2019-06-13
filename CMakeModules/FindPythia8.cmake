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

function (_PYTHIA8_CONFIG_ option variable type doc)
  execute_process(COMMAND ${PYTHIA8_CONFIG} ${option}
    OUTPUT_VARIABLE _local_out_
    RESULT_VARIABLE _local_res_)
  string(REGEX REPLACE "\n$" "" _local_out_ "${_local_out_}")
  if (NOT ${_local_res_} EQUAL 0)
    message ("Error in running ${PYTHIA8_CONFIG} ${option}")
  else ()
    set (${variable} "${_local_out_}" CACHE ${type} ${doc})
  endif ()
endfunction (_PYTHIA8_CONFIG_)
  


#################################################
# 
# Searched PYTHIA8 on system. Expect pythia8-config in PATH, or typical installation location
#
# This module defines
# HAVE_PYTHIA8
# PYTHIA8_INCLUDE_DIR   where to locate Pythia.h file
# PYTHIA8_LIBRARY       where to find the libpythia8 library
# PYTHIA8_LIBRARIES     (not cached) the libraries to link against to use Pythia8
# PYTHIA8_VERSION       version of Pythia8 if found
#

set (_SEARCH_PYTHIA8_
  ${PROJECT_BINARY_DIR}/ThirdParty/pythia8-install
  ${PYTHIA8}
  $ENV{PYTHIA8}
  ${PYTHIA8DIR}
  $ENV{PYTHIA8DIR}
  ${PYTHIA8_DIR}
  $ENV{PYTHIA8_DIR}
  ${PYTHIA8_ROOT}
  $ENV{PYTHIA8_ROOT}
  /opt/pythia8)

find_program (PYTHIA8_CONFIG
  NAME pythia8-config
  PATHS ${_SEARCH_PYTHIA8_}
  PATH_SUFFIXES "/bin"
  DOC "The location of the pythia8-config script")

if (PYTHIA8_CONFIG)
  set (HAVE_PYTHIA8 1 CACHE BOOL "presence of pythia8, found via pythia8-config")

  _PYTHIA8_CONFIG_ ("--prefix" PYTHIA8_PREFIX PATH "location of pythia8 installation")
  _PYTHIA8_CONFIG_ ("--includedir" PYTHIA8_INCLUDE_DIR PATH "pythia8 include directory")
  _PYTHIA8_CONFIG_ ("--libs" PYTHIA8_LIBRARY STRING "the pythia8 libs")
  _PYTHIA8_CONFIG_ ("--datadir" PYTHIA8_DATA_DIR PATH "the pythia8 data dir")
  _PYTHIA8_CONFIG_ ("--cxxflags" PYTHIA8_CXXFLAGS STRING "the pythia8 cxxflags")
endif ()

# standard cmake infrastructure:
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Pythia8 DEFAULT_MSG PYTHIA8_PREFIX PYTHIA8_INCLUDE_DIR PYTHIA8_LIBRARY)
mark_as_advanced (PYTHIA8_DATA_DIR PYTHIA8_CXXFLAGS PYTHIA8_INCLUDE_DIR PYTHIA8_LIBRARY)
