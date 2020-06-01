#
# (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
#
# See file AUTHORS for a list of contributors.
#
# This software is distributed under the terms of the GNU General Public
# Licence version 3 (GPL Version 3). See file LICENSE for a full version of
# the license.
#

# - find Conex
#
# This module defines
# CONEX_FOUND
# CONEX_PREFIX
# CONEX_INCLUDE_DIR

set (SEARCH_conex_
  ${WITH_CONEX}
  ${CONEXROOT}
  ${CONEX_ROOT}
  $ENV{CONEXROOT}
  $ENV{CONEX_ROOT}
  )

find_path (CONEX_PREFIX
  NAMES lib/${CMAKE_SYSTEM_NAME}
  PATHS ${SEARCH_conex_}
  DOC "The CONEX root directory"
  NO_DEFAULT_PATH
  )

find_path (CONEX_INCLUDE_DIR
  NAMES ConexDynamicInterface.h
  PATHS ${CONEX_PREFIX}
  PATH_SUFFIXES src
  DOC "The CONEX include directory"
  )

find_library (CONEX_LIBRARY
  NAMES libCONEXdynamic.a
  PATHS ${CONEX_PREFIX}
  PATH_SUFFIXES lib/${CMAKE_SYSTEM_NAME}
  DOC "The CONEX library"
  )

if (CONEX_INCLUDE_DIR)
  if (NOT CONEX_FIND_QUIETLY)
    message (STATUS "Conex include directory is ${CONEX_INCLUDE_DIR}")
  endif ()
  
  set (CMAKE_REQUIRED_INCLUDES ${CONEX_INCLUDE_DIR})
  include (CheckCXXSourceCompiles)
  check_cxx_source_compiles (
    "
    #include <conexHEModels.h>
    
    int 
    main() 
    { 
      EHEModel test = eSibyll23; 
      if (test == eSibyll23) return 0;
      return 0; 
    }
    "
    HAS_SIBYLL23
    )
  
  check_cxx_source_compiles (
    "
    #include <conexHEModels.h>
    
    int 
    main() 
    { 
      EHEModel test = eEposLHC; 
      if (test == eEposLHC) return 0;
      return 0; 
    }
    " 
    HAS_EPOS_LHC
    )
  
  check_cxx_source_compiles (
    "
    #include <ConexDynamicInterface.h>
    #include <ConexDynamicInterface.cc>
    int 
    main() 
    {
      ConexDynamicInterface cdi(eSibyll23);
      cdi.GetLeadingInteractionsParticleData();
      return 0; 
    }
    " 
    HAS_LEADINGINTERACTION_INTERFACE
    )
  
  if (HAS_LEADINGINTERACTION_INTERFACE)
     # at least conex2r5.65
     set (CONEX_VERSION 565)
     if (NOT CONEX_FIND_QUIETLY)
       message (STATUS "Conex has interface to leading interactions. Set _CONEX2R_VERSION=565.")
     endif ()
  elseif (HAS_SIBYLL23)
     # at least conex2r5.30
     set (CONEX_VERSION 530)
     if (NOT CONEX_FIND_QUIETLY)
       message (STATUS "Conex has SIBYLL2.3. Set _CONEX2R_VERSION=530.")
     endif ()
  elseif (HAS_EPOS_LHC)
     # at least conex2r4.36
     set (CONEX_VERSION 436)
     if (NOT CONEX_FIND_QUIETLY)
       message (STATUS "Conex has EPOS-LHC. Set _CONEX2R_VERSION=436.")
     endif ()
  else ()
     # pre LHC
     set (CONEX_VERSION 300)
     if (NOT CONEX_FIND_QUIETLY)
       message (STATUS "Conex is pre-LHC. Set _CONEX2R_VERSION=300.")
     endif ()
  endif ()
endif ()

# standard cmake infrastructure:
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (CONEX
  "Did not find system-level CONEX."
  CONEX_INCLUDE_DIR CONEX_LIBRARY CONEX_VERSION)
mark_as_advanced (CONEX_INCLUDE_DIR CONEX_LIBRARY CONEX_VERSION)
