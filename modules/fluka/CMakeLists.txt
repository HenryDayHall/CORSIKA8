set(C8_FLUKALIB CACHE STRING "path to libflukahp.a")

if (DEFINED C8_FLUKALIB)
    add_library(flukahp STATIC IMPORTED)
    set_property(TARGET flukahp PROPERTY IMPORTED_LOCATION ${C8_FLUKALIB})
    find_library(MATH_LIBRARY m)
    target_link_libraries(flukahp INTERFACE gfortran ${MATH_LIBRARY})
    target_include_directories (flukahp INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/corsika_modules/fluka>
    )
    
    # add fluka to corsika8 build
    add_dependencies (CORSIKA8 flukahp)
    target_link_libraries (CORSIKA8 INTERFACE flukahp)
endif()
