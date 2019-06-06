set (
  TESTING_SOURCES
  TestMain.cc
  )

set (
  TESTING_HEADERS
  )

set (
  TESTING_NAMESPACE
  corsika/testing
  )

add_library (CORSIKAtesting STATIC ${TESTING_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAtesting ${TESTING_NAMESPACE} ${TESTING_HEADERS})

set_target_properties (
  CORSIKAtesting
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PUBLIC_HEADER "${TESTING_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  CORSIKAtesting
  CORSIKAthirdparty
  )

# target_include_directories (
#  CORSIKAtesting
#  SYSTEM
#  PUBLIC    ${CATCH2_INCLUDE_DIR}
#  INTERFACE ${CATCH2_INCLUDE_DIR}
#  )

target_include_directories (
  CORSIKAtesting
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS CORSIKAtesting
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${TESTING_NAMESPACE}
  )

