add_library (CORSIKAthirdparty INTERFACE)

target_include_directories (CORSIKAthirdparty SYSTEM
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include/ThirdParty>
  )

install (DIRECTORY phys DESTINATION include/ThirdParty/)
install (DIRECTORY catch2 DESTINATION include/ThirdParty/)

include(ExternalProject)

# eventually add AUTO here, too:
set (ThirdPartyChoiceValues "C8;SYSTEM" CACHE STRING
    "List of possible values for the ThirdParty package choice")
mark_as_advanced (ThirdPartyChoiceValues)

##############################################################################
# Build spdlog
#set (CMAKE_POSITION_INDEPENDENT_CODE ON) # this is needed for PROPOSAL... since it is dynamic
add_subdirectory (spdlog)
#unset (CMAKE_POSITION_INDEPENDENT_CODE)

##############################################################################
# check for boost: either use C8 or system-level installation

message ("***** Configuring boost version")

set (USE_BOOST_C8 "C8" CACHE STRING
    "Selection of boost package. Can be \'C8\' or \'SYSTEM\'. Default: \'C8\'.")
set_property (CACHE USE_BOOST_C8 PROPERTY STRINGS ${ThirdPartyChoiceValues} )
if (NOT (${USE_BOOST_C8} IN_LIST ThirdPartyChoiceValues))
  message (SEND_ERROR "Illegal USE_BOOST_C8=\"${USE_BOOST_C8}\" can only be one of: ${ThirdPartyChoiceValues}")
endif (NOT (${USE_BOOST_C8} IN_LIST ThirdPartyChoiceValues))
message (STATUS "USE_BOOST_C8='${USE_BOOST_C8}'")

add_library (C8::ext::boost INTERFACE IMPORTED GLOBAL)
if ("x_${USE_BOOST_C8}" STREQUAL "x_SYSTEM")
  find_package (Boost REQUIRED mp11 iterator core format interval optional type_index histogram)

  message (STATUS "Using system-level boost version ${Boost_VERSION} at ${Boost_INCLUDE_DIR}")
  set_target_properties (
    C8::ext::boost PROPERTIES
    INTERFACE_LINK_LIBRARIES Boost::headers
    )
  set (Boost_FOUND 1 PARENT_SCOPE)
  
else ()

  set (_C8_Boost_VERSION "107300")
  message (STATUS "Building ThirdParty/boost using boost-${_C8_Boost_VERSION}.tar.bz2")
  ExternalProject_Add (boost
    URL ${CMAKE_CURRENT_SOURCE_DIR}/boost-${_C8_Boost_VERSION}.tar.bz2
    URL_MD5 895e35757f133f01c451fc93229aae6b
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost/install/boost 
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost/install/boost 
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL FALSE
    )
  set (HAVE_Boost 1 CACHE BOOL "presence of boost, via external-project-add in ThirdParty folder")
  set (Boost_FOUND 1 PARENT_SCOPE)
  ExternalProject_Get_Property (boost INSTALL_DIR)
  set (Boost_VERSION ${_C8_Boost_VERSION} CACHE STRING "Version of Boost")
  set (Boost_PREFIX ${INSTALL_DIR}/..)
  set (Boost_INCLUDE_DIR  ${Boost_PREFIX})
  set (Boost_LIBRARY_DIR ${Boost_PREFIX})
  add_dependencies (C8::ext::boost boost)
  
  # create include directory at config time
  file (MAKE_DIRECTORY ${Boost_INCLUDE_DIR})
  
  set (Boost_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/externals/boost)
  install (DIRECTORY ${Boost_PREFIX}/ DESTINATION ${Boost_INSTALL_DIR})
  
  message (STATUS "Use ThirdParty boost include dir ${Boost_INCLUDE_DIR}")
  set_target_properties (
    C8::ext::boost PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
    )
 
endif ()


##############################################################################
# check for Eigen3: either use ThirdParty/eigen3 or system-level installation

message ("***** Configuring eigen3 version")

set (USE_EIGEN3_C8 "C8" CACHE STRING
    "Selection of eigen3 package. Can be \'C8\' or \'SYSTEM\'. Default: \'C8\'.")
set_property (CACHE USE_EIGEN3_C8 PROPERTY STRINGS ${ThirdPartyChoiceValues})
if (NOT (${USE_EIGEN3_C8} IN_LIST ThirdPartyChoiceValues))
  message (SEND_ERROR "Illegal USE_EIGEN3_C8=\"${USE_EIGEN3_C8}\" can only be one of: ${ThirdPartyChoiceValues}")
endif (NOT (${USE_EIGEN3_C8} IN_LIST ThirdPartyChoiceValues))
message (STATUS "USE_EIGEN3_C8='${USE_EIGEN3_C8}'")

add_library (C8::ext::eigen3 INTERFACE IMPORTED GLOBAL)
if ("x_${USE_EIGEN3_C8}" STREQUAL "x_SYSTEM")
  
  if (WITH_EIGEN3)
    list (APPEND CMAKE_MODULE_PATH "${WITH_EIGEN3}/cmake")
  endif (WITH_EIGEN3)
  find_package (Eigen3 REQUIRED NO_MODULE)
  message (STATUS "Using system-level eigen3 version ${Eigen3_VERSION} at ${Eigen3_INCLUDE_DIR}")
  set_target_properties (
    C8::ext::eigen3 PROPERTIES
    INTERFACE_LINK_LIBRARIES Eigen3::Eigen
  )
set (Eigen3_FOUND 1 PARENT_SCOPE)

else ()

  set (_C8_Eigen3_VERSION "eigen-eigen-b3f3d4950030")

  message (STATUS "Building ThirdParty/eigen3 using ${_C8_Eigen3_VERSION}.tar.bz2")
  ExternalProject_Add (eigen3
    URL ${CMAKE_CURRENT_SOURCE_DIR}/${_C8_Eigen3_VERSION}.tar.bz2
    URL_MD5 e83549a79d1b721da0f8899ab34edf95
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen3/install/eigen
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen3/install/eigen
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL FALSE
    )
  set (Eigen3_FOUND 1 PARENT_SCOPE)
  ExternalProject_Get_Property (eigen3 INSTALL_DIR)
  set (Eigen3_PREFIX ${INSTALL_DIR}/..)
  set (Eigen3_INCLUDE_DIR  ${Eigen3_PREFIX}/eigen)
  add_dependencies (C8::ext::eigen3 eigen3)

  # create include directory at config time
  file (MAKE_DIRECTORY ${Eigen3_INCLUDE_DIR})
  
  set (Eigen3_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/externals/eigen3)
  install (DIRECTORY ${Eigen3_PREFIX}/ DESTINATION ${Eigen3_INSTALL_DIR})
  
  message (STATUS "Use ThirdParty eigen3 include dir ${Eigen3_INCLUDE_DIR}")
  set_target_properties (
    C8::ext::eigen3 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${Eigen3_INCLUDE_DIR}>
    )
 
endif ()


##############################################################################
# check for Pythia8: either use C8 or system-level installation

message ("***** Configuring Pythia8 version")

set (USE_PYTHIA8_C8 "C8" CACHE STRING
    "Selection of pythia8 package. Can be \'C8\' or \'SYSTEM\'. Default: \'C8\'.")
set_property (CACHE USE_PYTHIA8_C8 PROPERTY STRINGS ${ThirdPartyChoiceValues})
if (NOT (${USE_PYTHIA8_C8} IN_LIST ThirdPartyChoiceValues))
  message (SEND_ERROR "Illegal USE_PYTHIA8_C8=\"${USE_PYTHIA8_C8}\" can only be one of: ${ThirdPartyChoiceValues}")
endif (NOT (${USE_PYTHIA8_C8} IN_LIST ThirdPartyChoiceValues))
message (STATUS "USE_PYTHIA8_C8='${USE_PYTHIA8_C8}'")

add_library (C8::ext::pythia8 STATIC IMPORTED GLOBAL)
if ("x_${USE_PYTHIA8_C8}" STREQUAL "x_SYSTEM")
  
  find_package (Pythia8 REQUIRED) 
  message (STATUS "Using system-level Pythia8 version ${Pythia8_VERSION} at ${Pythia8_INCLUDE_DIR}")
  set_target_properties (
    C8::ext::pythia8 PROPERTIES
    IMPORTED_LOCATION ${Pythia8_LIBRARY}
    IMPORTED_LINK_INTERFACE_LIBRARIES dl
    INTERFACE_INCLUDE_DIRECTORIES ${Pythia8_INCLUDE_DIR}
    )
  set (Pythia8_FOUND 1 PARENT_SCOPE)

else ()

  set (_C8_Pythia8_VERSION "8235")
  message (STATUS "Building ThirdParty/pythia8 using pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2")
  message (STATUS "This will take a bit.....")
  ExternalProject_Add (pythia8
    URL ${CMAKE_CURRENT_SOURCE_DIR}/pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2
    URL_MD5 83132880c0594b808bd7fd37fb642606
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythia8/source
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythia8/install
    CONFIGURE_COMMAND ./configure --cxx-common=-Wno-deprecated-copy --prefix=${CMAKE_CURRENT_BINARY_DIR}/pythia8/install
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL TRUE
    )
  set (Pythia8_FOUND 1 PARENT_SCOPE)
  ExternalProject_Get_Property (pythia8 INSTALL_DIR)
  set (Pythia8_VERSION ${_C8_Pythia8_VERSION} CACHE STRING "Version of Pythia8")
  set (Pythia8_PREFIX ${INSTALL_DIR})
  set (Pythia8_INCLUDE_DIR  ${Pythia8_PREFIX}/include)
  set (Pythia8_LIBRARY ${Pythia8_PREFIX}/lib/libpythia8.a)
  add_dependencies (C8::ext::pythia8 pythia8)

  # create include directory at config time
  file (MAKE_DIRECTORY ${Pythia8_INCLUDE_DIR})

  set (Pythia8_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/externals/pythia8)
  install (DIRECTORY ${INSTALL_DIR}/ DESTINATION ${Pythia8_INSTALL_DIR})
  
  set_target_properties (
    C8::ext::pythia8 PROPERTIES
    IMPORTED_LOCATION ${Pythia8_LIBRARY}
    IMPORTED_LINK_INTERFACE_LIBRARIES dl
    INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${Pythia8_INCLUDE_DIR}>
    )

endif ()



##############################################################################
# check for CxRoot/CONEX: either use ThirdParty/cxroot or system-level installation

message ("***** Configuring CxRoot/CONEX version")

set (USE_CONEX_C8 "C8" CACHE STRING
    "Selection of conex package. Can be \'C8\' or \'SYSTEM\'. Default: \'C8\'.")
set_property (CACHE USE_CONEX_C8 PROPERTY STRINGS ${ThirdPartyChoiceValues})
if (NOT (${USE_CONEX_C8} IN_LIST ThirdPartyChoiceValues))
  message (SEND_ERROR "Illegal USE_CONEX_C8=\"${USE_CONEX_C8}\" can only be one of: ${ThirdPartyChoiceValues}")
endif (NOT (${USE_CONEX_C8} IN_LIST ThirdPartyChoiceValues))
message (STATUS "USE_CONEX_C8='${USE_CONEX_C8}'")

add_library (C8::ext::conex STATIC IMPORTED GLOBAL)
# this is from the corsika-data/readLib submodule:
get_directory_property (Boost_IOSTREAMS_FOUND DIRECTORY ${CORSIKA_DATA}/readLib DEFINITION Boost_IOSTREAMS_FOUND)
set (conex_boost "")
set (boost_lib "boost_iostreams")
if (NOT Boost_IOSTREAMS_FOUND)
  set (conex_boost "NO_BOOST=1")
  set (boost_lib "")
endif (NOT Boost_IOSTREAMS_FOUND)
if ("x_${USE_CONEX_C8}" STREQUAL "x_SYSTEM")
  
  find_package (CONEX REQUIRED) 
  message (STATUS "Using system-level CONEX version at ${CONEX_INCLUDE_DIR}")
  set (CONEX_FOUND 1 PARENT_SCOPE)

else ()
  
  message (STATUS "Building conex obtained via git and compiled")
  message (STATUS "This will take a bit.....")
  if (${CMAKE_VERSION} VERSION_LESS "3.16.0")
    message (WARNING "You need cmake >= 3.16 to support proper external git submodules. Right now, it will be more inefficient than necessary.")
  else (${CMAKE_VERSION} VERSION_LESS "3.16.0")
    cmake_policy (SET CMP0097 NEW) # avoid cloning of git submodules
  endif (${CMAKE_VERSION} VERSION_LESS "3.16.0")
  ExternalProject_Add (cxroot
    GIT_REPOSITORY https://gitlab.ikp.kit.edu/AirShowerPhysics/cxroot.git
    GIT_SUBMODULES ""
    GIT_TAG origin/master
    GIT_SHALLOW 5
    GIT_PROGRESS 1
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cxroot/source
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/cxroot/source
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make CX_NO_ROOT=1 CORSIKA_8=1 ${conex_boost} CORSIKA_DATA=${CORSIKA_DATA} all
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL TRUE
    )
  set (CONEX_FOUND 1 PARENT_SCOPE)
  ExternalProject_Get_Property (cxroot INSTALL_DIR)
  get_filename_component(INSTALL_DIR_ABS ${INSTALL_DIR} ABSOLUTE)
  set (CONEX_PREFIX ${INSTALL_DIR_ABS})
  set (CONEX_INCLUDE_DIR  ${CONEX_PREFIX}/src)
  set (CONEX_INCLUDE_DIR  ${CONEX_PREFIX}/src PARENT_SCOPE)
  add_dependencies (C8::ext::conex cxroot)

  # create include directory at config time
  file (MAKE_DIRECTORY ${CONEX_INCLUDE_DIR})
    
endif ()

if (Boost_IOSTREAMS_FOUND)
  set_target_properties (
    C8::ext::conex PROPERTIES
    IMPORTED_LOCATION ${CONEX_PREFIX}/lib/${CMAKE_SYSTEM_NAME}/libCONEXsibyll.a
    IMPORTED_NO_SONAME 1
    SKIP_BUILD_RPATH FALSE
    IMPORTED_LINK_INTERFACE_LIBRARIES boost_iostreams
    INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CONEX_INCLUDE_DIR}>    
    )
else (Boost_IOSTREAMS_FOUND)
  set_target_properties (
    C8::ext::conex PROPERTIES
    IMPORTED_LOCATION ${CONEX_PREFIX}/lib/${CMAKE_SYSTEM_NAME}/libCONEXsibyll.a
    IMPORTED_NO_SONAME 1
    SKIP_BUILD_RPATH FALSE
    INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CONEX_INCLUDE_DIR}>    
    )
endif (Boost_IOSTREAMS_FOUND)
