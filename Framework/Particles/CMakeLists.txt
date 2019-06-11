set(Python_ADDITIONAL_VERSIONS 3)
find_package(PythonInterp 3 REQUIRED)

add_custom_command (
  OUTPUT  ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc
          ${PROJECT_BINARY_DIR}/Framework/Particles/particle_db.pkl
  COMMAND ${PROJECT_SOURCE_DIR}/Framework/Particles/pdxml_reader.py 
          ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleData.xml
          ${PROJECT_SOURCE_DIR}/Framework/Particles/NuclearData.xml
          ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleClassNames.xml
  DEPENDS pdxml_reader.py
          ParticleData.xml
          NuclearData.xml
          ParticleClassNames.xml
  WORKING_DIRECTORY
          ${PROJECT_BINARY_DIR}/Framework/Particles/
  COMMENT "Read PYTHIA8 particle data and produce C++ source code GeneratedParticleProperties.inc"
  VERBATIM
  )

set (
  PARTICLE_SOURCES
  ParticleProperties.cc
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

add_library (CORSIKAparticles STATIC ${PARTICLE_SOURCES})

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
add_custom_target (SourceDirLink DEPENDS ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc)
add_dependencies (CORSIKAparticles SourceDirLink)
# .....................................................

target_link_libraries (
  CORSIKAparticles
  PUBLIC
  CORSIKAunits
  )

set_target_properties (
  CORSIKAparticles
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PUBLIC_HEADER "${PARTICLE_HEADERS}"
  )

target_include_directories (
  CORSIKAparticles
  PUBLIC
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
CORSIKA_ADD_TEST(testParticles
  SOURCES
    testParticles.cc
    ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc
)
target_link_libraries (
  testParticles
  CORSIKAparticles
  CORSIKAunits
  CORSIKAtesting
  )
