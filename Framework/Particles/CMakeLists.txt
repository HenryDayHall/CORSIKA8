
add_custom_command (
  OUTPUT  ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc
  COMMAND ${PROJECT_SOURCE_DIR}/Framework/Particles/pdxml_reader.py 
          ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleData.xml
          ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleClassNames.xml
  DEPENDS pdxml_reader.py
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
  ParticleProperties.h
  ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc # this one is auto-generated
  )

set (
  PARTICLE_NAMESPACE
  corsika/particles
  )

add_library (CORSIKAparticles INTERFACE)

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAparticles ${PARTICLE_NAMESPACE} ${PARTICLE_HEADERS})

# ....................................................
# since GeneratedParticleProperties.inc is an automatically produced file in the build directory,
# create a symbolic link into the source tree, so that it can be found and edited more easily
# this is not needed for the build to succeed! .......
add_custom_command (
  OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/GeneratedParticleProperties.inc
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_BINARY_DIR}/include/corsika/particles/GeneratedParticleProperties.inc ${CMAKE_CURRENT_SOURCE_DIR}/GeneratedParticleProperties.inc
  COMMENT "Generate link in source-dir: ${CMAKE_CURRENT_SOURCE_DIR}/GeneratedParticleProperties.inc"
  )
add_custom_target (SourceDirLink DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/GeneratedParticleProperties.inc)
add_dependencies (CORSIKAparticles SourceDirLink)
# .....................................................


target_include_directories (
  CORSIKAparticles
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES
  ${PARTICLE_HEADERS}
  DESTINATION
  include/${PARTICLE_NAMESPACE}
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

