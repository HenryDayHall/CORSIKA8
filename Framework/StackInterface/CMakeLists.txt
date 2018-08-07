
add_library (CORSIKAstackinterface INTERFACE)

target_include_directories (CORSIKAstackinterface INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
                                                            $<INSTALL_INTERFACE:include/Framework>
                                                            )

install (FILES Stack.h StackIterator.h
         DESTINATION include/StackInterface)

