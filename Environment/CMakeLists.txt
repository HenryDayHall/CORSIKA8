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
  )

set (
  ENVIRONMENT_NAMESPACE
  corsika/environment
  )

add_library (CORSIKAenvironment INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAenvironment ${ENVIRONMENT_NAMESPACE} ${ENVIRONMENT_HEADERS})

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  CORSIKAenvironment
  INTERFACE
  CORSIKAgeometry
  CORSIKAparticles
  CORSIKAunits
  CORSIKArandom
  )

target_include_directories (
  CORSIKAenvironment
  INTERFACE
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
