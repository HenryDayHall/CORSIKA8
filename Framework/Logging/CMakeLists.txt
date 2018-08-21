

add_library (CORSIKAlogging INTERFACE)

target_include_directories (CORSIKAlogging INTERFACE ${PROJECT_SOURCE_DIR}/Framework)

target_include_directories (CORSIKAlogging INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
                                                     $<INSTALL_INTERFACE:include/Framework>
                                                     )

install (FILES Logger.h Sink.h MessageOn.h MessageOff.h NoSink.h Sink.h BufferedSink.h
         DESTINATION include/Logging)

# code testing
add_executable (testLogging testLogging.cc)
target_link_libraries (testLogging CORSIKAlogging CORSIKAthirdparty) # for catch2
add_test (NAME testLogging COMMAND testLogging -o report.xml -r junit)

