set (GeometryNodeStackExtension_HEADERS GeometryNodeStackExtension.h)
set (GeometryNodeStackExtension_NAMESPACE corsika/stack/node)

add_library (GeometryNodeStackExtension INTERFACE)

CORSIKA_COPY_HEADERS_TO_NAMESPACE (GeometryNodeStackExtension ${GeometryNodeStackExtension_NAMESPACE} ${GeometryNodeStackExtension_HEADERS})

target_link_libraries (
  GeometryNodeStackExtension
  INTERFACE
  CORSIKAstackinterface
  CORSIKAunits
  CORSIKAparticles
  CORSIKAgeometry
  )

target_include_directories (
  GeometryNodeStackExtension
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES
  ${GeometryNodeStackExtension_HEADERS}
  DESTINATION
  include/${GeometryNodeStackExtension_NAMESPACE}
  )

# ----------------
# code unit testing
 CORSIKA_ADD_TEST(testGeometryNodeStackExtension)
 target_link_libraries (
   testGeometryNodeStackExtension
   GeometryNodeStackExtension
   CORSIKAtesting
   )
