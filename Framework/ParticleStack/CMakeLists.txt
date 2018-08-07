
set (STACK_HEADERS StackOne.h)

add_library (CORSIKAstack INTERFACE)

#set_target_properties (CORSIKAstack PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties (CORSIKAstack PROPERTIES SOVERSION 1)

#set_target_properties (CORSIKAstack PROPERTIES PUBLIC_HEADER "${STACK_HEADERS}")

#target_link_libraries (CORSIKAstackinterface CORSIKAunits)

#target_include_directories (CORSIKAstack PRIVATE   ${EIGEN3_INCLUDE_DIR})
target_include_directories (CORSIKAstack INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
                                                      $<INSTALL_INTERFACE:include/Framework>
                                                      )

install (FILES StackOne.h DESTINATION include/Stack)
