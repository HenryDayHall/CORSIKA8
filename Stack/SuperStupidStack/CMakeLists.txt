set (SuperStupidStack_HEADERS SuperStupidStack.h)
set (SuperStupidStack_NAMESPACE corsika/stack/super_stupid)

add_library (SuperStupidStack INTERFACE)

CORSIKA_COPY_HEADERS_TO_NAMESPACE (SuperStupidStack ${SuperStupidStack_NAMESPACE} ${SuperStupidStack_HEADERS})

target_link_libraries (
  SuperStupidStack
  INTERFACE
  CORSIKAstackinterface
  CORSIKAunits
  CORSIKAparticles
  CORSIKAgeometry
  )

target_include_directories (
  SuperStupidStack
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES
  ${SuperStupidStack_HEADERS}
  DESTINATION
  include/${SuperStupidStack_NAMESPACE}
  )

# ----------------
# code unit testing
CORSIKA_ADD_TEST(testSuperStupidStack)
target_link_libraries (
  testSuperStupidStack
  CORSIKAgeometry
  CORSIKAparticles
  CORSIKAunits
  CORSIKAtesting
  )
