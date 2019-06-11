add_library (CORSIKAlogging INTERFACE)

# namespace of library -> location of header files
set (
  CORSIKAlogging_NAMESPACE
  corsika/logging
  )

# header files of this library
set (
  CORSIKAlogging_HEADERS
  Logger.h
  Sink.h
  MessageOn.h
  MessageOff.h
  NoSink.h
  Sink.h
  BufferedSink.h
  )

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAlogging ${CORSIKAlogging_NAMESPACE} ${CORSIKAlogging_HEADERS})

# include directive for upstream code
target_include_directories (
  CORSIKAlogging
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  ${Boost_INCLUDE_DIRS}
  )

# install library
install (
  FILES ${CORSIKAlogging_HEADERS}
  DESTINATION include/${CORSIKAlogging_NAMESPACE}
  )

# ----------------
# code unit testing

#CORSIKA_ADD_TEST (testLogging)
#target_link_libraries (
#  testLogging
#  CORSIKAlogging
#  CORSIKAtesting
#  )

