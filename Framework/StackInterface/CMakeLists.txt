set (
  CORSIKAstackint_HEADERS
  Stack.h
  StackIterator.h
  )

set (
  CORSIKAstackint_NAMESPACE
  fwk
  )

add_library (
  CORSIKAstackint
  INTERFACE
  )

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAstackint ${CORSIKAstackint_NAMESPACE} ${CORSIKAstackint_HEADERS})

target_include_directories (
  CORSIKAstackint
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

#add_custom_command (
#  TARGET
#  CORSIKAstackinterface
#  PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CORSIKAstackinterface_HEADERS} ${PROJECT_BINARY_DIR}/include/corsika
#  )

install (
  FILES
  ${CORSIKAstackint_HEADERS}
  DESTINATION
  include/${CORSIKAstackint_NAMESPACE}
  )
