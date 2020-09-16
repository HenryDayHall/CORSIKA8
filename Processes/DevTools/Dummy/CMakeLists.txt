add_library (DevTools_Dummy INTERFACE)

set (
  DevTools_Dummy_HEADERS
  DummyBoundaryCrossingProcess.h
  DummyContinuousProcess.h
  DummyDecayProcess.h
  DummyInteractionProcess.h
  DummySecondariesProcess.h  
  )

set (
  DevTools_Dummy_NAMESPACE
  corsika/process/devtools
  )

CORSIKA_COPY_HEADERS_TO_NAMESPACE (DevTools_Dummy ${DevTools_Dummy_NAMESPACE} ${DevTools_Dummy_HEADERS})


target_include_directories (
  DevTools_Dummy 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  FILES ${DevTools_Dummy_HEADERS}
  DESTINATION include/${DevTools_Dummy_NAMESPACE}
  )


# --------------------
# code unit testing
CORSIKA_ADD_TEST (TestDummy)


target_link_libraries (
  TestDummy  
  DevTools_Dummy
  CORSIKAunits
  CORSIKAstackinterface
  CORSIKAprocesssequence
  CORSIKAsetup
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAtesting
  )
