set (NuclearStackExtension_HEADERS NuclearStackExtension.h)
set (NuclearStackExtension_NAMESPACE corsika/stack/nuclear_extension)

add_library (NuclearStackExtension INTERFACE)

CORSIKA_COPY_HEADERS_TO_NAMESPACE (NuclearStackExtension ${NuclearStackExtension_NAMESPACE} ${NuclearStackExtension_HEADERS})

target_link_libraries (
  NuclearStackExtension
  INTERFACE
  CORSIKAstackinterface
  CORSIKAunits
  CORSIKAparticles
  CORSIKAgeometry
  )

target_include_directories (
  NuclearStackExtension
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install (
  FILES
  ${NuclearStackExtension_HEADERS}
  DESTINATION
  include/${NuclearStackExtension_NAMESPACE}
  )

# ----------------
# code unit testing
CORSIKA_ADD_TEST(testNuclearStackExtension)
target_link_libraries (
  testNuclearStackExtension
  SuperStupidStack
  NuclearStackExtension
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  )
