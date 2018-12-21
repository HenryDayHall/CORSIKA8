
set (
  UTILITIES_SOURCES  
  COMBoost.cc
  )

set (
  UTILITIES_HEADERS
  COMBoost.h
  Bit.h
  Singleton.h
  )

set (
  UTILITIES_NAMESPACE
  corsika/utl
  )

add_library (CORSIKAutilities STATIC ${UTILITIES_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAutilities ${UTILITIES_NAMESPACE} ${UTILITIES_HEADERS})

set_target_properties (
  CORSIKAutilities
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PUBLIC_HEADER "${UTILITIES_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  CORSIKAutilities
  CORSIKAgeometry
  CORSIKAunits
  )

target_include_directories (
  CORSIKAutilities
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS CORSIKAutilities
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${UTILITIES_NAMESPACE}
  )


# --------------------
# code unit testing
add_executable (testCOMBoost testCOMBoost.cc)

target_link_libraries (
  testCOMBoost
  CORSIKAutilities
  CORSIKAthirdparty # for catch2
  )

add_test (NAME testCOMBoost COMMAND testCOMBoost)

