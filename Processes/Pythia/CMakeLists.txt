set(Python_ADDITIONAL_VERSIONS 3)
find_package(PythonInterp 3 REQUIRED)
  
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
  corsika/process/pythia
  )

add_library (ProcessPythia STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessPythia ${MODEL_NAMESPACE} ${MODEL_HEADERS})


set_target_properties (
  ProcessPythia
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessPythia
  CORSIKAparticles
  CORSIKAutilities
  CORSIKAunits
  CORSIKAthirdparty
  CORSIKAgeometry
  CORSIKAenvironment
  ${PYTHIA8_LIBRARY}
  )

target_include_directories (
  ProcessPythia
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

target_include_directories (
  ProcessPythia
  SYSTEM
  PUBLIC ${PYTHIA8_INCLUDE_DIR}
  INTERFACE ${PYTHIA8_INCLUDE_DIR}
  )

install (
  TARGETS ProcessPythia
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing

CORSIKA_ADD_TEST(testPythia
  SOURCES
  testPythia.cc
  ${MODEL_HEADERS}
)
target_link_libraries (
  testPythia
  ProcessPythia
  CORSIKAsetup
  CORSIKArandom
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  ${PYTHIA8_LIBRARY}
  )

