
add_library (CORSIKAlogging INTERFACE)

# namespace of library -> location of header files
set (
  CORSIKAlogging_NAMESPACE
  fwk
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
  )

# install library
install (
  FILES ${CORSIKAlogging_HEADERS}
  DESTINATION include/${CORSIKAlogging_NAMESPACE}
  )

# ----------------
# code unit testing
add_executable (
  testLogging
  testLogging.cc
  )

target_link_libraries (
  testLogging
  CORSIKAlogging
  CORSIKAthirdparty # for catch2
  )

add_test (
  NAME testLogging
  COMMAND testLogging
  )

