set (
  CORSIKAstackinterface_HEADERS
  Stack.h
  StackIterator.h
  ParticleBase.h
  )

set (
  CORSIKAstackinterface_NAMESPACE
  corsika/stack
  )

add_library (
  CORSIKAstackinterface
  INTERFACE
  )

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAstackinterface ${CORSIKAstackinterface_NAMESPACE} ${CORSIKAstackinterface_HEADERS})

target_include_directories (
  CORSIKAstackinterface
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES
  ${CORSIKAstackinterface_HEADERS}
  DESTINATION
  include/${CORSIKAstackinterface_NAMESPACE}
  )

#code testing
add_executable (testStackInterface testStackInterface.cc)
add_test(NAME testStackInterface COMMAND testStackInterface)
target_link_libraries (testStackInterface CORSIKAstackinterface CORSIKAthirdparty) # for catch2
