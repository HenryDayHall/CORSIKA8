set (
  MODEL_SOURCES
  ExecTime.cc
  )

set (
  MODEL_HEADERS
  ExecTime.h
  ImplBoundary.h
  ImplContinuous.h
  ImplDecay.h
  ImplInteraction.h
  ImplSecondaries.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/devtools
  )

add_library (DevTools_Analytic STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (DevTools_Analytic ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  DevTools_Analytic
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  DevTools_Analytic
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAthirdparty  
  )

target_include_directories (
  DevTools_Analytic 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS DevTools_Analytic
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib 
  )


# --------------------
# code unit testing
CORSIKA_ADD_TEST (testExecTime
 SOURCES
 testExecTime.cc
 ${MODEL_HEADERS}
)

target_link_libraries (
  testExecTime  
  DevTools_Analytic  
  DevTools_Dummy
  CORSIKAtesting
  CORSIKAthirdparty # for catch2
  )

