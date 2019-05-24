set (
  SETUP_HEADERS
  SetupStack.h
  SetupLogger.h
  SetupEnvironment.h
  SetupTrajectory.h
  )

set (
  SETUP_NAMESPACE
  corsika/setup
  )

add_library (CORSIKAsetup INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAsetup ${SETUP_NAMESPACE} ${SETUP_HEADERS})

target_link_libraries (
  CORSIKAsetup
  INTERFACE
  CORSIKAgeometry
  SuperStupidStack
  NuclearStackExtension
  )

target_include_directories (
  CORSIKAsetup
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  FILES ${SETUP_HEADERS} 
  DESTINATION include/${SETUP_NAMESPACE}
  )
