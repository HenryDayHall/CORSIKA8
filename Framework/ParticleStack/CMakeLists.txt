
set (PUBLIC_HEADER_FILES StackOne.h)

add_library (CORSIKAstack INTERFACE)

#set_target_properties (CORSIKAstack PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties (CORSIKAstack PROPERTIES SOVERSION 1)

#set_target_properties (CORSIKAstack PROPERTIES PUBLIC_HEADER "${STACK_HEADERS}")

#target_link_libraries (CORSIKAstackinterface CORSIKAunits)

#target_include_directories (CORSIKAstack PRIVATE   ${EIGEN3_INCLUDE_DIR})
target_include_directories (
  CORSIKAstack
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

#add_custom_command (
#  CORSIKAstack
#  PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} ${PROJECT_BINARY_DIR}/include/corsika})
#  )

install (
  FILES
  StackOne.h
  DESTINATION
  include/Stack
  )
