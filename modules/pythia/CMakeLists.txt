include (ExternalProject)

# eventually add AUTO here, too:
set (ThirdPartyChoiceValues "C8;SYSTEM" CACHE STRING
    "List of possible values for the ThirdParty package choice")
mark_as_advanced (ThirdPartyChoiceValues)

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
    IMPORTED_LOCATION ${Pythia8_LIBRARY}/libpythia8.a
    IMPORTED_LINK_INTERFACE_LIBRARIES dl
    INTERFACE_INCLUDE_DIRECTORIES ${Pythia8_INCLUDE_DIR}
    )
  set (Pythia8_FOUND 1 PARENT_SCOPE)

  message ("Pythia8_LIBRARY=${Pythia8_LIBRARY}")
  message ("Pythia8_INCLUDE_DIR=${Pythia8_INCLUDE_DIR}")

else ()

  set (_C8_Pythia8_VERSION "8235")
  message (STATUS "Building modules/pythia8 using pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2")
  message (STATUS "This will take a bit.....")
  ExternalProject_Add (pythia8
    URL ${CMAKE_CURRENT_SOURCE_DIR}/pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2
    URL_MD5 83132880c0594b808bd7fd37fb642606
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythia8/source
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/external/pythia8
    LOG_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythia8/log
    CONFIGURE_COMMAND ./configure --cxx-common=\\"-Wall -pedantic -Wextra -Wno-ignored-qualifiers -O0 -DNDEBUG  -Wno-nonportable-include-path -g -std=c++17 -Wno-deprecated-copy -fPIC\\" --prefix=${CMAKE_INSTALL_PREFIX}/external/pythia8
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL TRUE
    LOG_CONFIGURE TRUE
    LOG_BUILD TRUE
    LOG_INSTALL TRUE
    LOG_MERGED_STDOUTERR TRUE
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

    message (STATUS "Pythia8_INCLUDE_DIR ${Pythia8_INCLUDE_DIR}")
endif ()


##### add pythia8 to CORSIKA8 build
add_dependencies (CORSIKA8 C8::ext::pythia8)
target_link_libraries (CORSIKA8 INTERFACE C8::ext::pythia8)

