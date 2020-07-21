# create the library
add_library (CORSIKAlogging INTERFACE)

# namespace of library -> location of header files
set (
  CORSIKAlogging_NAMESPACE
  corsika/logging
  )

# header files of this library
set (
  CORSIKAlogging_HEADERS
  Logging.h
  )

# copy the headers into the namespace
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAlogging
  ${CORSIKAlogging_NAMESPACE}
  ${CORSIKAlogging_HEADERS}
  )

# include directive for upstream code
target_include_directories (
  CORSIKAlogging
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  C8::ext:boost
  )

# and link against spdlog
target_link_libraries(
  CORSIKAlogging
  INTERFACE
  spdlog::spdlog
)

# install library
install (
  FILES ${CORSIKAlogging_HEADERS}
  DESTINATION include/${CORSIKAlogging_NAMESPACE}
  )

# ----------------
# code unit testing

CORSIKA_ADD_TEST (testLogging)
target_link_libraries (
 testLogging
 CORSIKAlogging
 CORSIKAtesting
 )
