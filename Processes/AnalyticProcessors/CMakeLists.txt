set (
  MODEL_SOURCES  
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
  corsika/process/analytic_processors
  )

add_library (AnalyticProcessors INTERFACE)  #STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (AnalyticProcessors ${MODEL_NAMESPACE} ${MODEL_HEADERS})

target_link_libraries(AnalyticProcessors 
  INTERFACE
  CORSIKAlogging)

target_include_directories (
  AnalyticProcessors 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS AnalyticProcessors
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
  AnalyticProcessors
  ExampleProcessors
  CORSIKAtesting
  C8::ext::eigen3
  CORSIKAthirdparty # for catch2  
  )

