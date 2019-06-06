set (
  MODEL_SOURCES
  StackInspector.cc
  )

set (
  MODEL_HEADERS
  StackInspector.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/stack_inspector
  )

add_library (ProcessStackInspector STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessStackInspector ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessStackInspector
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessStackInspector
  CORSIKAcascade
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAsetup
  CORSIKAlogging
  )

target_include_directories (
  ProcessStackInspector 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessStackInspector
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
CORSIKA_ADD_TEST (testStackInspector)
target_link_libraries (
  testStackInspector
  ProcessStackInspector
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  )
