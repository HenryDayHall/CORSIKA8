add_library (CORSIKAunits INTERFACE)

set (CORSIKAunits_NAMESPACE corsika/units)
set (
  CORSIKAunits_HEADERS
  PhysicalUnits.h
  PhysicalConstants.h
  )

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAunits ${CORSIKAunits_NAMESPACE} ${CORSIKAunits_HEADERS})

target_include_directories (CORSIKAunits
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include>
  )

install (FILES ${CORSIKAunits_HEADERS} DESTINATION include/${CORSIKAunits_NAMESPACE})

# code testing
CORSIKA_ADD_TEST (testUnits)
target_link_libraries (testUnits CORSIKAunits CORSIKAtesting)
