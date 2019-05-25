set (DummyStack_HEADERS DummyStack.h)
set (DummyStack_NAMESPACE corsika/stack/dummy)

add_library (DummyStack INTERFACE)

CORSIKA_COPY_HEADERS_TO_NAMESPACE (DummyStack ${DummyStack_NAMESPACE} ${DummyStack_HEADERS})

target_link_libraries (
  DummyStack
  INTERFACE
  CORSIKAstackinterface
  CORSIKAunits
  CORSIKAparticles
  )

target_include_directories (
  DummyStack
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES
  ${DummyStack_HEADERS}
  DESTINATION
  include/${DummyStack_NAMESPACE}
  )
