set (
  MODEL_SOURCES
  Decay.cc
  Random.cc
  Interaction.cc
  )

set (
  MODEL_HEADERS
  Decay.h
  Random.h
  Interaction.h
  )

set (
  MODEL_NAMESPACE
  dependencies/Pythia
  )

add_library (ProcessPythia8 STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessPythia8 ${MODEL_NAMESPACE} ${MODEL_HEADERS})


set_target_properties (
  ProcessPythia8
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessPythia8
  CORSIKAprocesssequence
  CORSIKAparticles
  CORSIKAutilities
  CORSIKAunits
  CORSIKAthirdparty
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAsetup
  CORSIKArandom
  C8::ext::pythia8
  )

target_include_directories (
  ProcessPythia8
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessPythia8
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )


# --------------------
# code unit testing

CORSIKA_ADD_TEST(testPythia8
  SOURCES
  testPythia8.cc
  ${MODEL_HEADERS}
)
target_link_libraries (
  testPythia8
  ProcessPythia8
  CORSIKAtesting
  )

