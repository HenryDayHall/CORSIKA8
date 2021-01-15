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
  string (REGEX REPLACE "\n$" "" _local_out_ "${_local_out_}")
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
# HAVE_Pythia8
# Pythia8_INCLUDE_DIR   where to locate Pythia.h file
# Pythia8_LIBRARY       where to find the libpythia8 library
# Pythia8_LIBRARIES     (not cached) the libraries to link against to use Pythia8
# Pythia8_VERSION       version of Pythia8 if found
#

set (_SEARCH_Pythia8_
  ${PROJECT_BINARY_DIR}/ThirdParty/pythia8-install
  ${PYTHIA8}
  $ENV{PYTHIA8}
  ${PYTHIA8DIR}
  $ENV{PYTHIA8DIR}
  ${PYTHIA8_ROOT}
  $ENV{PYTHIA8_ROOT}
  ${PYTHIA8_DIR}
  $ENV{PYTHIA8_DIR}
  /opt/pythia8
  )

find_program (Pythia8_CONFIG
  NAME pythia8-config
  PATHS ${_SEARCH_Pythia8_}
  PATH_SUFFIXES "/bin"
  DOC "The location of the pythia8-config script")

if (Pythia8_CONFIG)
  set (HAVE_Pythia8 1 CACHE BOOL "presence of pythia8, found via pythia8-config")

  _Pythia8_CONFIG_ ("--prefix" Pythia8_PREFIX PATH "location of pythia8 installation")
  _Pythia8_CONFIG_ ("--includedir" Pythia8_INCLUDE_DIR PATH "pythia8 include directory")
  _Pythia8_CONFIG_ ("--libdir" Pythia8_LIBRARY STRING "the pythia8 libs")
  _Pythia8_CONFIG_ ("--datadir" Pythia8_DATA_DIR PATH "the pythia8 data dir")
  _Pythia8_CONFIG_ ("--cxxflags" Pythia8_CXXFLAGS STRING "the pythia8 cxxflags")
endif ()

# standard cmake infrastructure:
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Pythia8 DEFAULT_MSG Pythia8_PREFIX Pythia8_INCLUDE_DIR Pythia8_LIBRARY)
mark_as_advanced (Pythia8_DATA_DIR Pythia8_CXXFLAGS Pythia8_INCLUDE_DIR Pythia8_LIBRARY)
