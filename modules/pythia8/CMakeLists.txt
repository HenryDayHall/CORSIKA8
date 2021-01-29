include (ExternalProject)

# eventually add AUTO here, too:
set (ThirdPartyChoiceValues "C8;SYSTEM" CACHE STRING
    "List of possible values for the ThirdParty package choice")
mark_as_advanced (ThirdPartyChoiceValues)

##############################################################################
# check for Pythia8: either use C8 or system-level installation

message (STATUS "***** Configuring Pythia8 version")

set (USE_Pythia8_C8 "C8" CACHE STRING
    "Selection of pythia8 package. Can be \'C8\' or \'SYSTEM\'. Default: \'C8\'.")
set_property (CACHE USE_Pythia8_C8 PROPERTY STRINGS ${ThirdPartyChoiceValues})
if (NOT (${USE_Pythia8_C8} IN_LIST ThirdPartyChoiceValues))
  message (SEND_ERROR "Illegal USE_Pythia8_C8=\"${USE_Pythia8_C8}\" can only be one of: ${ThirdPartyChoiceValues}")
endif (NOT (${USE_Pythia8_C8} IN_LIST ThirdPartyChoiceValues))
message (STATUS "USE_Pythia8_C8='${USE_Pythia8_C8}'")

set (CORSIKA_Pythia8_MODULE_DIR ${PROJECT_BINARY_DIR}/corsika/modules/pythia8)
file (MAKE_DIRECTORY ${CORSIKA_Pythia8_MODULE_DIR})

add_library (C8::ext::pythia8 STATIC IMPORTED GLOBAL)
if ("x_${USE_Pythia8_C8}" STREQUAL "x_SYSTEM")
  
  find_package (Pythia8 8245 EXACT REQUIRED) 
  message (STATUS "Using system-level Pythia8 version ${Pythia8_VERSION} at ${Pythia8_PREFIX}")
  set (Pythia8_INCLUDE_DIRS ${Pythia8_INCLUDE_DIR})
  set_target_properties (
    C8::ext::pythia8 PROPERTIES
    IMPORTED_LOCATION ${Pythia8_LIBRARY}/libpythia8.a
    IMPORTED_LINK_INTERFACE_LIBRARIES dl
    INTERFACE_INCLUDE_DIRECTORIES "${Pythia8_INCLUDE_DIRS}"
    )
  set (Pythia8_FOUND 1 PARENT_SCOPE)

  set (CORSIKA_Pythia8_LOCATION ${Pythia8_PREFIX}) # build and install location, pythia8 is external
  configure_file (Pythia8ConfigurationDirectory.hpp.in ${CORSIKA_Pythia8_MODULE_DIR}/Pythia8ConfigurationDirectory.hpp  @ONLY)
  configure_file (Pythia8ConfigurationDirectory.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/Pythia8ConfigurationDirectory_install.hpp  @ONLY)
  
  set (Pythia8_PREFIX_INSTALL ${Pythia8_PREFIX}) # install and build are both the same here
  
  message (STATUS "Pythia8 (system) at ${Pythia8_PREFIX}")

else ()

  set (_C8_Pythia8_VERSION "8245")
  message (STATUS "Building modules/pythia8 using pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2")
  message (STATUS "This will take a bit.....")
  # this is not a full Pythia8 install, it is a bit simplified, e.g. no pythia8-config
  ExternalProject_Add (pythia8
    URL ${CMAKE_CURRENT_SOURCE_DIR}/pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2
    URL_MD5 d3e951a2f101e8cfec26405cb61db83b
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/pythia8/source"
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/pythia8/install"
    CONFIGURE_COMMAND ./configure --cxx-common=\\"${CMAKE_CXX_FLAGS} -O2 -fPIC\\" --prefix=${CMAKE_CURRENT_BINARY_DIR}/pythia8/install
    INSTALL_COMMAND make install COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/pythia8/install/bin ${CMAKE_CURRENT_BINARY_DIR}/pythia8/install/share/Pythia8/examples
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL TRUE
    )  
  set (Pythia8_FOUND 1 PARENT_SCOPE)
  set (Pythia8_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/externals/pythia8)
  set (Pythia8_PREFIX_INSTALL ${Pythia8_INSTALL_DIR})
  ExternalProject_Get_Property (pythia8 INSTALL_DIR) # this is "make"
  set (Pythia8_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/externals/pythia8) # this is "make install"
  set (Pythia8_VERSION ${_C8_Pythia8_VERSION} CACHE STRING "Version of Pythia8")
  set (Pythia8_PREFIX ${INSTALL_DIR})
  set (Pythia8_INCLUDE_DIR  ${Pythia8_PREFIX}/include)
  set (Pythia8_LIBRARY ${Pythia8_PREFIX}/lib)
  add_dependencies (C8::ext::pythia8 pythia8)

  set (CORSIKA_Pythia8_LOCATION ${Pythia8_PREFIX}) # build location
  configure_file (Pythia8ConfigurationDirectory.hpp.in ${CORSIKA_Pythia8_MODULE_DIR}/Pythia8ConfigurationDirectory.hpp  @ONLY)
  set (CORSIKA_Pythia8_LOCATION ${Pythia8_INSTALL_DIR}) # install location
  configure_file (Pythia8ConfigurationDirectory.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/Pythia8ConfigurationDirectory_install.hpp  @ONLY)
  
  install (DIRECTORY ${INSTALL_DIR}/ DESTINATION ${Pythia8_INSTALL_DIR})

  set (Pythia8_INCLUDE_DIRS ${Pythia8_INCLUDE_DIR})
  
  set_target_properties (
    C8::ext::pythia8
    PROPERTIES
    IMPORTED_LOCATION ${Pythia8_LIBRARY}/libpythia8.a
    IMPORTED_LINK_INTERFACE_LIBRARIES dl
    INTERFACE_INCLUDE_DIRECTORIES "${Pythia8_INCLUDE_DIRS}"
    )
  
    message (STATUS "Pythia8 build at: ${INSTALL_DIR}, final install to: ${Pythia8_INSTALL_DIR}, include dirs: ${Pythia8_INCLUDE_DIRS}")
endif ()

# the install location of Pythia8
install (
  FILES ${CMAKE_CURRENT_BINARY_DIR}/Pythia8ConfigurationDirectory_install.hpp
  RENAME Pythia8ConfigurationDirectory.hpp
  DESTINATION include/corsika/modules/pythia8
  )

##### add pythia8 to CORSIKA8 build
add_dependencies (CORSIKA8 C8::ext::pythia8)
target_link_libraries (CORSIKA8 INTERFACE C8::ext::pythia8)

# we need those for exporting targets and dependencies:
set (Pythia8_FOUND 1 PARENT_SCOPE)
set (Pythia8_PREFIX ${Pythia8_PREFIX} PARENT_SCOPE)
set (Pythia8_PREFIX_INSTALL ${Pythia8_PREFIX_INSTALL} PARENT_SCOPE)
