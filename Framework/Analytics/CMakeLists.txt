# create the library
add_library (CORSIKAanalytics INTERFACE)

# namespace of library -> location of header files
set (
  CORSIKAanalytics_NAMESPACE
  corsika/analytics
  )

# header files of this library
set (
  CORSIKAanalytics_HEADERS
  ClassTimer.h
  FunctionTimer.h
  )

# copy the headers into the namespace
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAanalytics
  ${CORSIKAanalytics_NAMESPACE}
  ${CORSIKAanalytics_HEADERS}
  )

# include directive for upstream code
target_include_directories (
  CORSIKAanalytics
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  C8::ext:boost
  )

# and link against spdlog
#target_link_libraries(
#  CORSIKAanalytics
#  INTERFACE
#  spdlog::spdlog
#)

# install library
install (
  FILES ${CORSIKAanalytics_HEADERS}
  DESTINATION include/${CORSIKAanalytics_NAMESPACE}
  )

# ----------------
# code unit testing

<<<<<<< HEAD
CORSIKA_ADD_TEST (testFunctionTimer)
target_link_libraries (
 testFunctionTimer
 CORSIKAanalytics
 CORSIKAtesting
 )

 CORSIKA_ADD_TEST (testClassTimer)
target_link_libraries (
 testClassTimer
=======
CORSIKA_ADD_TEST (testAnalytics)
target_link_libraries (
 testAnalytics
>>>>>>> New timer class added for cleaner structure
 CORSIKAanalytics
 CORSIKAtesting
 )
