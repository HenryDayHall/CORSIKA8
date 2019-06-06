set(Python_ADDITIONAL_VERSIONS 3)
find_package(PythonInterp 3 REQUIRED)

add_custom_command (
  OUTPUT  ${PROJECT_BINARY_DIR}/Processes/Sibyll/Generated.inc
  COMMAND ${PROJECT_SOURCE_DIR}/Processes/Sibyll/code_generator.py 
          ${PROJECT_BINARY_DIR}/Framework/Particles/particle_db.pkl
          ${PROJECT_SOURCE_DIR}/Processes/Sibyll/sibyll_codes.dat
  DEPENDS code_generator.py
          sibyll_codes.dat          
          ${PROJECT_BINARY_DIR}/Framework/Particles/particle_db.pkl
  WORKING_DIRECTORY
          ${PROJECT_BINARY_DIR}/Processes/Sibyll/
  COMMENT "Generate conversion tables for particle codes SIBYLL <-> CORSIKA"
  VERBATIM
  )
  
set (
  MODEL_SOURCES
  ParticleConversion.cc
  Interaction.cc
  Decay.cc
  NuclearInteraction.cc
  sibyll2.3c.f
  nuclib.f
  signuc.f
  sibyll2.3c.cc
  gasdev.f
  )

set (
  MODEL_HEADERS
  ParticleConversion.h
  sibyll2.3c.h
  nuclib.h
  SibStack.h
  Decay.h
  Interaction.h
  NuclearInteraction.h
  ${PROJECT_BINARY_DIR}/Processes/Sibyll/Generated.inc
  )

set (
  MODEL_NAMESPACE
  corsika/process/sibyll
  )

add_library (ProcessSibyll STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessSibyll ${MODEL_NAMESPACE} ${MODEL_HEADERS})

# ....................................................
# since Generated.inc is an automatically produced file in the build directory,
# create a symbolic link into the source tree, so that it can be found and edited more easily
# this is not needed for the build to succeed! .......
add_custom_command (
  OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/Generated.inc
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_BINARY_DIR}/include/corsika/process/sibyll/Generated.inc ${CMAKE_CURRENT_SOURCE_DIR}/Generated.inc
  COMMENT "Generate link in source-dir: ${CMAKE_CURRENT_SOURCE_DIR}/Generated.inc"
  )
add_custom_target (SourceDirLink2 DEPENDS ${PROJECT_BINARY_DIR}/Processes/Sibyll/Generated.inc)
add_dependencies (ProcessSibyll SourceDirLink2)
# .....................................................



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
  CORSIKAprocesssequence
  CORSIKAparticles
  CORSIKAutilities
  CORSIKAunits
  CORSIKAthirdparty
  CORSIKAgeometry
  CORSIKAenvironment
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
CORSIKA_ADD_TEST(testSibyll
  SOURCES
  testSibyll.cc
  ${MODEL_HEADERS}
)

target_link_libraries (
  testSibyll
  ProcessSibyll
  CORSIKAsetup
  CORSIKArandom
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  )
