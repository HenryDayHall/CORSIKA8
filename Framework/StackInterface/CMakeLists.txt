set (
  CORSIKAstackinterface_HEADERS
  Stack.h
  StackIterator.h
  )

set (
  CORSIKAstackinterface_NAMESPACE
  fwk
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

#add_custom_command (
#  TARGET
#  CORSIKAstackinterfaceerface
#  PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CORSIKAstackinterfaceerface_HEADERS} ${PROJECT_BINARY_DIR}/include/corsika
#  )

install (
  FILES
  ${CORSIKAstackinterface_HEADERS}
  DESTINATION
  include/${CORSIKAstackinterface_NAMESPACE}
  )
