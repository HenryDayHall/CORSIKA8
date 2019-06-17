set (
  GEOMETRY_SOURCES
  CoordinateSystem.cc
  )

set (
  GEOMETRY_HEADERS
  Vector.h
  Point.h
  Line.h
  Sphere.h
  Plane.h
  Volume.h
  CoordinateSystem.h
  RootCoordinateSystem.h
  Helix.h
  BaseVector.h
  QuantityVector.h
  Trajectory.h
  FourVector.h
  )

set (
  GEOMETRY_NAMESPACE
  corsika/geometry
  )

add_library (CORSIKAgeometry STATIC ${GEOMETRY_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAgeometry ${GEOMETRY_NAMESPACE} ${GEOMETRY_HEADERS})

set_target_properties (
  CORSIKAgeometry
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PUBLIC_HEADER "${GEOMETRY_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAutilities
  )

target_include_directories (
  CORSIKAgeometry
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

target_include_directories (
  CORSIKAgeometry
  SYSTEM
  PUBLIC "${EIGEN3_INCLUDE_DIR}"
  )

install (
  TARGETS CORSIKAgeometry
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${GEOMETRY_NAMESPACE}
  )

# --------------------
# code unit testing
CORSIKA_ADD_TEST(testGeometry)
target_link_libraries (
  testGeometry
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  )

CORSIKA_ADD_TEST(testFourVector)
target_link_libraries (
  testFourVector
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  )
