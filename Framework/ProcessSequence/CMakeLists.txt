
add_library (CORSIKAprocesssequence INTERFACE)

target_include_directories (CORSIKAprocesssequence INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
                                                   $<INSTALL_INTERFACE:include/Framework>
                                                   )

install (FILES ProcessSequence.h
         DESTINATION include/ProcessSequence)

