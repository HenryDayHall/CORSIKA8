set (
  ExampleProcessors_HEADERS
  DummyBoundaryCrossingProcess.h
  DummyContinuousProcess.h
  DummyDecayProcess.h
  DummyInteractionProcess.h
  DummySecondariesProcess.h  
  )

set (
  ExampleProcessors_NAMESPACE
  corsika/process/example_processors
  )

add_library (ExampleProcessors INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ExampleProcessors ${ExampleProcessors_NAMESPACE} ${ExampleProcessors_HEADERS})


target_include_directories (
  ExampleProcessors 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  FILES ${ExampleProcessors_HEADERS}
  DESTINATION include/${ExampleProcessors_NAMESPACE}
  )


# --------------------
# code unit testing
CORSIKA_ADD_TEST (TestDummy)


target_link_libraries (
  TestDummy  
  ExampleProcessors
  CORSIKAunits
  CORSIKAstackinterface
  CORSIKAprocesssequence
  CORSIKAsetup
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAtesting
  )
