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
target_link_libraries (testStackInterface CORSIKAstackinterface CORSIKAthirdparty) # for catch2
CORSIKA_ADD_TEST(testStackInterface)

add_executable (testSecondaryView testSecondaryView.cc)
target_link_libraries (testSecondaryView CORSIKAstackinterface CORSIKAthirdparty) # for catch2
CORSIKA_ADD_TEST(testSecondaryView)

add_executable (testCombinedStack testCombinedStack.cc)
target_link_libraries (testCombinedStack CORSIKAstackinterface CORSIKAthirdparty) # for catch2
CORSIKA_ADD_TEST(testCombinedStack)
