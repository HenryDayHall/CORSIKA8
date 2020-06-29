set (
  ENVIRONMENT_SOURCES
  LayeredSphericalAtmosphereBuilder.cc
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
  )

set (
  ENVIRONMENT_NAMESPACE
  corsika/environment
  )

add_library (CORSIKAenvironment STATIC ${ENVIRONMENT_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAenvironment ${ENVIRONMENT_NAMESPACE} ${ENVIRONMENT_HEADERS})

set_target_properties (
  CORSIKAgeometry
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PUBLIC_HEADER "${ENVIRONMENT_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  CORSIKAenvironment
  CORSIKAgeometry
  CORSIKAparticles
  CORSIKAunits
  CORSIKArandom
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
