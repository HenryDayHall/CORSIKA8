
set (GEOMETRY_SOURCES CoordinateSystem.cc)
set (GEOMETRY_HEADERS Vector.h Point.h CoordinateSystem.h)

add_library (CORSIKAgeometry STATIC ${GEOMETRY_SOURCES})

set_target_properties (CORSIKAgeometry PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties (CORSIKAgeometry PROPERTIES SOVERSION 1)

set_target_properties (CORSIKAgeometry PROPERTIES PUBLIC_HEADER "${GEOMETRY_HEADERS}")

target_include_directories (CORSIKAgeometry PRIVATE ${EIGEN3_INCLUDE_DIR})
target_include_directories (CORSIKAgeometry PRIVATE ${PROJECT_SOURCE_DIR}/Framework)

install (TARGETS CORSIKAgeometry
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PUBLIC_HEADER DESTINATION include/Geometry)
