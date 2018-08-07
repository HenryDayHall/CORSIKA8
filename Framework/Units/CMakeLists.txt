
add_library (CORSIKAunits INTERFACE)

target_include_directories (CORSIKAunits INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
                                                   $<INSTALL_INTERFACE:include/Framework>
                                                   )

install (FILES PhysicalUnits.h PhysicalConstants.h
         DESTINATION include/Units)

install (DIRECTORY units
         DESTINATION include/Units)
