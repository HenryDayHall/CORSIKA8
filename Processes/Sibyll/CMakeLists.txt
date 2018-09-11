
set (
  MODEL_SOURCES
  ParticleConversion.cc
  )

set (
  MODEL_HEADERS
  ParticleConversion.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/sibyll
  )

add_library (ProcessSibyll STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessSibyll ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessSibyll
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessSibyll
  CORSIKAunits
  )

target_include_directories (
  ProcessSibyll 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessSibyll
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
add_executable (testSibyll testSibyll.cc)

target_link_libraries (
  testSibyll
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAthirdparty # for catch2
  )

add_test (NAME testSibyll COMMAND testSibyll)

