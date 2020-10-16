add_custom_command (
  OUTPUT  ${PROJECT_BINARY_DIR}/Environment/GeneratedMediaProperties.inc
  COMMAND ${PROJECT_SOURCE_DIR}/Environment/readProperties.py 
          ${PROJECT_SOURCE_DIR}/Environment/properties8.dat
  DEPENDS readProperties.py
          properties8.dat
  WORKING_DIRECTORY
          ${PROJECT_BINARY_DIR}/Environment
  COMMENT "Read NIST properties8 data file and produce C++ source code GeneratedMediaProperties.inc"
  VERBATIM
  )


set (
  ENVIRONMENT_SOURCES
  ShowerAxis.cc
)

set (
  ENVIRONMENT_HEADERS
  VolumeTreeNode.h
  IMediumModel.h
  NuclearComposition.h
  HomogeneousMedium.h
  InhomogeneousMedium.h
  HomogeneousMedium.h
  LinearApproximationIntegrator.h
  DensityFunction.h
  Environment.h
  NameModel.h
  BaseExponential.h
  FlatExponential.h
  SlidingPlanarExponential.h
  LayeredSphericalAtmosphereBuilder.h
  ShowerAxis.h
  IMagneticFieldModel.h
  UniformMagneticField.h
  IRefractiveIndexModel.h
  UniformRefractiveIndex.h
  IMediumPropertyModel.h
  MediumProperties.h
  MediumPropertyModel.h
  ${PROJECT_BINARY_DIR}/Environment/GeneratedMediaProperties.inc # this one is auto-generated
  )

set (
  ENVIRONMENT_NAMESPACE
  corsika/environment
  )

add_library (CORSIKAenvironment STATIC ${ENVIRONMENT_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAenvironment ${ENVIRONMENT_NAMESPACE} ${ENVIRONMENT_HEADERS})

set_target_properties (
  CORSIKAenvironment
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PUBLIC_HEADER "${ENVIRONMENT_HEADERS}"
  )

# ....................................................
# since GeneratedMediaProperties.inc is an automatically produced file in the build directory,
# create a symbolic link into the source tree, so that it can be found and edited more easily
# this is not needed for the build to succeed! .......
add_custom_command (
  OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/GeneratedParticleProperties.inc
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_BINARY_DIR}/include/corsika/environment/GeneratedMediaProperties.inc ${CMAKE_CURRENT_SOURCE_DIR}/GeneratedMediaProperties.inc
  COMMENT "Generate link in source-dir: ${CMAKE_CURRENT_SOURCE_DIR}/GeneratedMediaProperties.inc"
  )
add_custom_target (SourceDirLinkMedia DEPENDS ${PROJECT_BINARY_DIR}/Environment/GeneratedMediaProperties.inc)
add_dependencies (CORSIKAenvironment SourceDirLinkMedia)
# .....................................................

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  CORSIKAenvironment
  CORSIKAgeometry
  CORSIKAparticles
  CORSIKAunits
  CORSIKAlogging
  )

target_include_directories (
  CORSIKAenvironment
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  TARGETS CORSIKAenvironment
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${ENVIRONMENT_NAMESPACE}
  )

# --------------------
# code unit testing
CORSIKA_ADD_TEST(testEnvironment)
target_link_libraries (
  testEnvironment
  CORSIKAsetup
  CORSIKAenvironment
  CORSIKAtesting
  )

CORSIKA_ADD_TEST(testShowerAxis)
target_link_libraries (
  testShowerAxis
  CORSIKAsetup
  CORSIKAenvironment
  CORSIKAtesting
  )
