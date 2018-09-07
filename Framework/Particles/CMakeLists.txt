
add_custom_command (
  OUTPUT ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc
  COMMAND ${PROJECT_SOURCE_DIR}/Framework/Particles/pdxml_reader.py ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleData.xml ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleClassNames.xml
  DEPENDS
  pdxml_reader.py
  ParticleData.xml
  ParticleClassNames.xml
  WORKING_DIRECTORY
  ${PROJECT_BINARY_DIR}/Framework/Particles/
  COMMENT "Read PYTHIA8 particle data and produce C++ source code GeneratedParticleProperties.inc"
  VERBATIM
  )

# all public header files of library, includes automatic generated file(s)
set (
  PARTICLE_HEADERS
  Particles.h
  ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc # this one is auto-generated
  )

# we need 2nd list with just the header files from the source code directory
set (
  PARTICLE_HEADERS_SOURCE
  Particles.h
  )

set (
  PARTICLE_NAMESPACE
  fwk
  )

add_library (CORSIKAparticles INTERFACE)

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAparticles ${PARTICLE_NAMESPACE} ${PARTICLE_HEADERS})

target_include_directories (
  CORSIKAparticles
  INTERFACE
  # ${EIGEN3_INCLUDE_DIR}
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES
  ${PARTICLE_HEADERS}
  DESTINATION
  include/Particles
  )

# --------------------
# code unit testing
add_executable (
  testParticles
  testParticles.cc
  ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc
  )

target_link_libraries (
  testParticles
  CORSIKAparticles
  CORSIKAunits
  CORSIKAthirdparty # for catch2
  )

add_test (
  NAME
  testParticles
  COMMAND
  testParticles
  )

