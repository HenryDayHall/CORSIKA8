# - find Conex
#
# This module defines
# CONEX_FOUND
# CONEX_ROOT_DIR
# CONEX_INCLUDE_DIR

FIND_PATH (CONEX_ROOT_DIR
  NAMES lib/${CMAKE_SYSTEM_NAME}
  PATHS $ENV{CONEXROOT}
  DOC "The CONEX root directory"
  NO_DEFAULT_PATH
)

FIND_PATH (CONEX_INCLUDE_DIR
  NAMES ConexDynamicInterface.h
  PATHS ${CONEX_ROOT_DIR}/src
  DOC "The CONEX include directory"
)

IF (CONEX_INCLUDE_DIR)
  SET (CONEX_FOUND TRUE)
  ADD_DEFINITIONS (-D_CONEX_PREFIX=\"${CONEX_ROOT_DIR}\" -D_CONEX_SYSTEM=\"${CMAKE_SYSTEM_NAME}\")
  IF (NOT Conex_FIND_QUIETLY)
    MESSAGE (STATUS "Conex include directory is ${CONEX_INCLUDE_DIR}")
  ENDIF ()
  SET (CMAKE_REQUIRED_INCLUDES ${CONEX_INCLUDE_DIR})
  CHECK_CXX_SOURCE_COMPILES (
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
  CHECK_CXX_SOURCE_COMPILES (
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
  CHECK_CXX_SOURCE_COMPILES (
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
  IF (HAS_LEADINGINTERACTION_INTERFACE)
     # at least conex2r5.65
     ADD_DEFINITIONS (-D_CONEX2R_VERSION=565)
     IF (NOT Conex_FIND_QUIETLY)
       MESSAGE (STATUS "Conex has interface to leading interactions. Set _CONEX2R_VERSION=565.")
     ENDIF ()
  ELSEIF (HAS_SIBYLL23)
     # at least conex2r5.30
     ADD_DEFINITIONS (-D_CONEX2R_VERSION=530)
     IF (NOT Conex_FIND_QUIETLY)
       MESSAGE (STATUS "Conex has SIBYLL2.3. Set _CONEX2R_VERSION=530.")
     ENDIF ()
  ELSEIF (HAS_EPOS_LHC)
     # at least conex2r4.36
     ADD_DEFINITIONS (-D_CONEX2R_VERSION=436)
     IF (NOT Conex_FIND_QUIETLY)
       MESSAGE (STATUS "Conex has EPOS-LHC. Set _CONEX2R_VERSION=436.")
     ENDIF ()
  ELSE ()
     # pre LHC
     ADD_DEFINITIONS (-D_CONEX2R_VERSION=300)     
     IF (NOT Conex_FIND_QUIETLY)
       MESSAGE (STATUS "Conex is pre-LHC. Set _CONEX2R_VERSION=300.")
     ENDIF ()
  ENDIF ()
ELSE ()
  SET (CONEX_FOUND FALSE)
  IF (Conex_FIND_REQUIRED)
    MESSAGE (FATAL_ERROR "Could not find Conex!")
  ENDIF ()
ENDIF ()
