set (
  MODEL_SOURCES
  CONEX.cc
  )

set (
  MODEL_HEADERS
  CONEX.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/conex
  )

add_library (ProcessConex STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessConex ${MODEL_NAMESPACE} ${MODEL_HEADERS})


set_target_properties (
  ProcessConex
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessConex
  CORSIKAprocesssequence
  CORSIKAparticles
  CORSIKAutilities
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAsetup
  CORSIKArandom
  C8::ext::conex
  )

target_include_directories (
  ProcessConex
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessConex
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )


# --------------------
# code unit testing

CORSIKA_ADD_TEST(testConex
  SOURCES
  testConex.cc
  ${MODEL_HEADERS}
)
target_link_libraries (
  testConex
  ProcessConex
  CORSIKAtesting
  )

