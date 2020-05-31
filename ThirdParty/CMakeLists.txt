include(ExternalProject)

add_library (CORSIKAthirdparty INTERFACE)

target_include_directories (CORSIKAthirdparty SYSTEM
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include/ThirdParty>
  )

install (DIRECTORY phys DESTINATION include/ThirdParty/)
install (DIRECTORY catch2 DESTINATION include/ThirdParty/)

# boost

add_library (CORSIKAboost INTERFACE IMPORTED GLOBAL)
if (NOT Boost_FOUND)
  set (_C8_Boost_VERSION "107300")
  message ("Building ThirdParty/boost using boost-${_C8_Boost_VERSION}.tar.bz2")
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
  add_dependencies (CORSIKAboost boost)
  
  # create include directory at config time
  file (MAKE_DIRECTORY ${Boost_INCLUDE_DIR})
  
  set (Boost_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/externals/boost)
  install (DIRECTORY ${Boost_PREFIX}/ DESTINATION ${Boost_INSTALL_DIR})
  
  message ("Use ThirdParty boost include dir ${Boost_INCLUDE_DIR}")
  set_target_properties (
    CORSIKAboost PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
    )
 
else (NOT Boost_FOUND)

  message ("Using system-level boost version ${Boost_VERSION} at ${Boost_INCLUDE_DIR}")
  set_target_properties (
    CORSIKAboost PROPERTIES
    INTERFACE_LINK_LIBRARIES Boost::headers
    )
  
endif (NOT Boost_FOUND)

# eigen3

add_library (CORSIKAeigen3 INTERFACE IMPORTED GLOBAL)
if (NOT Eigen3_FOUND)
  set (_C8_Eigen3_VERSION "eigen-eigen-b3f3d4950030")

  message ("Building ThirdParty/eigen3 using ${_C8_Eigen3_VERSION}.tar.bz2")
  ExternalProject_Add (eigen3
    URL ${CMAKE_CURRENT_SOURCE_DIR}/${_C8_Eigen3_VERSION}.tar.bz2
    URL_MD5 8ffed5895a851894ad866888d4e64685
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen3/install/eigen
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen3/install/eigen
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL FALSE
    )
  set (HAVE_Eigen3 1 CACHE BOOL "presence of eigen3, via external-project-add in ThirdParty folder")
  set (Eigen3_FOUND 1 PARENT_SCOPE)
  ExternalProject_Get_Property (eigen3 INSTALL_DIR)
  set (Eigen3_PREFIX ${INSTALL_DIR}/..)
  set (Eigen3_INCLUDE_DIR  ${Eigen3_PREFIX})
  set (Eigen3_LIBRARY_DIR ${Eigen3_PREFIX})
  add_dependencies (CORSIKAeigen3 eigen3)

  # create include directory at config time
  file (MAKE_DIRECTORY ${Eigen3_INCLUDE_DIR})
  
  set (Eigen3_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/externals/eigen3)
  install (DIRECTORY ${Eigen3_PREFIX}/ DESTINATION ${Eigen3_INSTALL_DIR})
  
  message ("Use ThirdParty eigen3 include dir ${Eigen3_INCLUDE_DIR}")
  set_target_properties (
    CORSIKAeigen3 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${Eigen3_INCLUDE_DIR}>
    )
 
else (NOT Eigen3_FOUND)

  message ("Using system-level eigen3 version ${Eigen3_VERSION} at ${Eigen3_INCLUDE_DIR}")
  set_target_properties (
    CORSIKAeigen3 PROPERTIES
    INTERFACE_LINK_LIBRARIES Eigen3::Eigen
  )
  
endif (NOT Eigen3_FOUND)

# Pythia8

add_library (CORSIKApythia8 STATIC IMPORTED GLOBAL)
if (NOT Pythia8_FOUND)
  set (_C8_Pythia8_VERSION "8235")
  message ("Building ThirdParty/pythia8 using pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2")
  message ("This will take a bit.....")
  ExternalProject_Add (pythia8
    URL ${CMAKE_CURRENT_SOURCE_DIR}/pythia${_C8_Pythia8_VERSION}-stripped.tar.bz2
    URL_MD5 83132880c0594b808bd7fd37fb642606
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythia8/source
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythia8/install
    CONFIGURE_COMMAND ./configure --cxx-common=-Wno-deprecated-copy --prefix=${CMAKE_CURRENT_BINARY_DIR}/pythia8/install
    BUILD_IN_SOURCE ON
    EXCLUDE_FROM_ALL TRUE
    )
  set (HAVE_Pythia8 1 CACHE BOOL "presence of pythia8, via external-project-add in ThirdParty folder")
  set (Pythia8_FOUND 1 PARENT_SCOPE)
  ExternalProject_Get_Property (pythia8 INSTALL_DIR)
  set (Pythia8_VERSION ${_C8_Pythia8_VERSION} CACHE STRING "Version of Pythia8")
  set (Pythia8_PREFIX ${INSTALL_DIR})
  set (Pythia8_INCLUDE_DIR  ${Pythia8_PREFIX}/include)
  set (Pythia8_LIBRARY ${Pythia8_PREFIX}/lib/libpythia8.a)
  add_dependencies (CORSIKApythia8 pythia8)

  # create include directory at config time
  file (MAKE_DIRECTORY ${Pythia8_INCLUDE_DIR})

  set (Pythia8_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/externals/pythia8)
  install (DIRECTORY ${INSTALL_DIR}/ DESTINATION ${Pythia8_INSTALL_DIR})
  
  set_target_properties (
    CORSIKApythia8 PROPERTIES
    IMPORTED_LOCATION ${Pythia8_LIBRARY}
    IMPORTED_LINK_INTERFACE_LIBRARIES dl
    INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${Pythia8_INCLUDE_DIR}>
    )

else (NOT Pythia8_FOUND)

  message ("Using system-level Pythia8 version ${Pythia8_VERSION} at ${Pythia8_INCLUDE_DIR}")
  set_target_properties (
    CORSIKApythia8 PROPERTIES
    IMPORTED_LOCATION ${Pythia8_LIBRARY}
    IMPORTED_LINK_INTERFACE_LIBRARIES dl
    INTERFACE_INCLUDE_DIRECTORIES ${Pythia8_INCLUDE_DIR}
    )
  
endif (NOT Pythia8_FOUND)

