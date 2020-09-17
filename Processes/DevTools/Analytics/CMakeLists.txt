set (
  MODEL_SOURCES
  ExecTime.cc
  )

set (
  MODEL_HEADERS
  ExecTime.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/devtools
  )

add_library (ProcessDevTools STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessDevTools ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessDevTools
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessDevTools
  CORSIKAsetup
  )

target_include_directories (
  ProcessDevTools 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessDevTools
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
CORSIKA_ADD_TEST (testExecTime testExecTime.cc)
target_link_libraries (
  testExecTime  ProcessDevTools
  CORSIKAsetup
  DevTools_Dummy
  CORSIKAthirdparty # for catch2
  )

