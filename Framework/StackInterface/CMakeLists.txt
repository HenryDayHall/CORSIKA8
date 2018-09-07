set (
  PUBLIC_HEADER_FILES
  Stack.h
  StackIterator.h
  )

add_library (
  CORSIKAstackinterface
  INTERFACE
  )

target_include_directories (
  CORSIKAstackinterface
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

#add_custom_command (
#  TARGET
#  CORSIKAstackinterface
#  PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} ${PROJECT_BINARY_DIR}/include/corsika
#  )

install (
  FILES
  ${PUBLIC_HEADER_FILES}
  DESTINATION
  include/corsika
  )
