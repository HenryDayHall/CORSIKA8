set (
  MODEL_SOURCES
  ParticleCut.cc
)

set (
  MODEL_HEADERS
  ParticleCut.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/particle_cut
  )

add_library (ProcessParticleCut STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessParticleCut ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessParticleCut
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessParticleCut
  CORSIKAunits
  CORSIKAparticles
  CORSIKAsetup
  )

target_include_directories (
  ProcessParticleCut 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessParticleCut
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )

# --------------------
# code unit testing
CORSIKA_ADD_TEST(testParticleCut SOURCES
  testParticleCut.cc
  ${MODEL_HEADERS}
)

target_link_libraries (
  testParticleCut
  ProcessParticleCut
  CORSIKAunits
  CORSIKAstackinterface
  CORSIKAprocesssequence
  CORSIKAsetup
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAtesting
  )
