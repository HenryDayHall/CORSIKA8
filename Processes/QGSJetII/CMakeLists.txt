set(Python_ADDITIONAL_VERSIONS 3)
find_package(PythonInterp 3 REQUIRED)

if (NOT DEFINED ENV{CORSIKA_DATA})
  message (WARNING "WARNING: download corsika-data repository from gitlab for needed data tables, and set CORSIKA_DATA path!")
endif ()

add_custom_command (
  OUTPUT  ${PROJECT_BINARY_DIR}/Processes/QGSJetII/Generated.inc
  COMMAND ${PROJECT_SOURCE_DIR}/Processes/QGSJetII/code_generator.py 
          ${PROJECT_BINARY_DIR}/Framework/Particles/particle_db.pkl
          ${PROJECT_SOURCE_DIR}/Processes/QGSJetII/qgsjet-II-04-codes.dat
  DEPENDS code_generator.py
          qgsjet-II-04-codes.dat
          ${PROJECT_BINARY_DIR}/Framework/Particles/particle_db.pkl
  WORKING_DIRECTORY
          ${PROJECT_BINARY_DIR}/Processes/QGSJetII/
  COMMENT "Generate conversion tables for particle codes QGSJetII <-> CORSIKA"
  VERBATIM
  )
  
set (
  MODEL_SOURCES
  ParticleConversion.cc
  Interaction.cc
  qgsjet-II-04.f
  qgsjet-II-04.cc
  )

set (
  MODEL_HEADERS
  ParticleConversion.h
  qgsjet-II-04.h
  QGSJetIIStack.h
  QGSJetIIFragmentsStack.h
  Interaction.h
  ${PROJECT_BINARY_DIR}/Processes/QGSJetII/Generated.inc
  )

set (
  MODEL_NAMESPACE
  corsika/process/qgsjetII
  )

add_library (ProcessQGSJetII STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessQGSJetII ${MODEL_NAMESPACE} ${MODEL_HEADERS})

# ....................................................
# since Generated.inc is an automatically produced file in the build directory,
# create a symbolic link into the source tree, so that it can be found and edited more easily
# this is not needed for the build to succeed! .......
add_custom_command (
  OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/Generated.inc
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_BINARY_DIR}/include/corsika/process/qgsjetII/Generated.inc ${CMAKE_CURRENT_SOURCE_DIR}/Generated.inc
  COMMENT "Generate link in source-dir: ${CMAKE_CURRENT_SOURCE_DIR}/Generated.inc"
  )
add_custom_target (SourceDirLinkQgsII DEPENDS ${PROJECT_BINARY_DIR}/Processes/QGSJetII/Generated.inc)
add_dependencies (ProcessQGSJetII SourceDirLinkQgsII)
# .....................................................



set_target_properties (
  ProcessQGSJetII
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessQGSJetII
  CORSIKAprocesssequence
  CORSIKAparticles
  CORSIKAutilities
  CORSIKAunits
  CORSIKAthirdparty
  CORSIKAgeometry
  CORSIKAenvironment
  )

target_include_directories (
  ProcessQGSJetII 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessQGSJetII
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

# install(
#    FILES qgsdat-II-04 sectnu-II-04 DESTINATION etc/data
# )


# --------------------
# code unit testing
CORSIKA_ADD_TEST(testQGSJetII
  SOURCES
  testQGSJetII.cc
  ${MODEL_HEADERS}
)

target_link_libraries (
  testQGSJetII
  ProcessQGSJetII
  CORSIKAsetup
  CORSIKArandom
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  )

# also provide QGSJetII large data tables for testing in build tree (links)
# -> changed that to use environment variable "${CORSIKA_DATA}/QGSJetII"
# add_custom_command (
#  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/qgsdat-II-04 ${CMAKE_CURRENT_BINARY_DIR}/sectnu-II-04
#  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/qgsdat-II-04 ${CMAKE_CURRENT_BINARY_DIR}/qgsdat-II-04
#  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/sectnu-II-04 ${CMAKE_CURRENT_BINARY_DIR}/sectnu-II-04
#  COMMENT "Generate link in source-dir: qgsjet-II-04"
#  )

# add_custom_target (QGSJetDataLink DEPENDS
#  ${CMAKE_CURRENT_BINARY_DIR}/qgsdat-II-04
#  ${CMAKE_CURRENT_BINARY_DIR}/sectnu-II-04
#  )

# add_dependencies (testQGSJetII QGSJetDataLink)
