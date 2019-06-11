set (
  CORSIKAstackinterface_HEADERS
  ParticleBase.h
  StackIteratorInterface.h
  Stack.h
  SecondaryView.h
  CombinedStack.h
  )

set (
  CORSIKAstackinterface_NAMESPACE
  corsika/stack
  )

add_library (
  CORSIKAstackinterface
  INTERFACE
  )

CORSIKA_COPY_HEADERS_TO_NAMESPACE (
  CORSIKAstackinterface ${CORSIKAstackinterface_NAMESPACE} ${CORSIKAstackinterface_HEADERS}
  )

target_include_directories (
  CORSIKAstackinterface
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES ${CORSIKAstackinterface_HEADERS}
  DESTINATION include/${CORSIKAstackinterface_NAMESPACE}
  )

#code testing
CORSIKA_ADD_TEST(testStackInterface)
target_link_libraries (testStackInterface CORSIKAstackinterface CORSIKAtesting)

CORSIKA_ADD_TEST(testSecondaryView)
target_link_libraries (testSecondaryView CORSIKAstackinterface CORSIKAtesting)

CORSIKA_ADD_TEST(testCombinedStack)
target_link_libraries (testCombinedStack CORSIKAstackinterface CORSIKAtesting)
