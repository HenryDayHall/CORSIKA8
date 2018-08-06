

add_library (CORSIKAlogging INTERFACE)

target_include_directories (CORSIKAlogging INTERFACE ${PROJECT_SOURCE_DIR}/Framework)

target_include_directories (CORSIKAlogging INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
                                                     $<INSTALL_INTERFACE:include/Framework>
                                                     )

install (FILES Logger.h
         DESTINATION include/Logging)

