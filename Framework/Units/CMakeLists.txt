
add_library (CORSIKAunits INTERFACE)

target_include_directories (CORSIKAunits
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include>
  )

install (FILES PhysicalUnits.h DESTINATION include/Units)

# code testing
add_executable (testUnits testUnits.cc)
target_link_libraries (testUnits CORSIKAunits CORSIKAthirdparty) # for catch2
add_test(NAME testUnits COMMAND testUnits)

