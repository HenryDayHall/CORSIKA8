
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
  Volume.h
  CoordinateSystem.h
  RootCoordinateSystem.h
  Helix.h
  BaseVector.h
  QuantityVector.h
  Trajectory.h
  # BaseTrajectory.h
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
  )

target_include_directories (
  CORSIKAgeometry
  PUBLIC    ${EIGEN3_INCLUDE_DIR}
  INTERFACE ${EIGEN3_INCLUDE_DIR}
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS CORSIKAgeometry
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${GEOMETRY_NAMESPACE}
  )


# --------------------
# code unit testing
add_executable (testGeometry testGeometry.cc)

target_link_libraries (
  testGeometry
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAthirdparty # for catch2
  )

add_test (NAME testGeometry COMMAND testGeometry)

