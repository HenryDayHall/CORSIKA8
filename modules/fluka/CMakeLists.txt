set(C8_FLUKALIB CACHE STRING "path to libflukahp.a")

if (DEFINED C8_FLUKALIB)
    set (input_dir ${PROJECT_SOURCE_DIR}/src/modules/fluka)
    
    # we remove flrndm_.o from the original libflukahp.a and save the result as libflukahp-norndm.a
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/libflukahp-norndm.a COMMAND ${input_dir}/strip_flukahp.sh ${C8_FLUKALIB} ${CMAKE_BINARY_DIR})
    add_custom_target(generate_libfluka-norndm DEPENDS ${CMAKE_BINARY_DIR}/libflukahp-norndm.a)

    add_library(flukahp-norndm STATIC IMPORTED)
    add_dependencies(flukahp-norndm generate_libfluka-norndm)
    set_property(TARGET flukahp-norndm PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libflukahp-norndm.a)
    find_library(MATH_LIBRARY m)
    target_link_libraries(flukahp-norndm INTERFACE gfortran ${MATH_LIBRARY})
    target_include_directories (flukahp-norndm INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/corsika_modules/fluka>
    )
    
    # build shared library containing FLUKA + our own implementation of flrndm_()
    # Note: This apparently works only when built as shared lib, not possible so easily as static lib
    # Note also that this lib is not self-contained as it doesn't contain fluka::rndm_interface(), so
    # you can't load it at runtime
    add_library(fluka SHARED fluka.cpp)
    target_link_libraries(fluka PRIVATE flukahp-norndm)

    target_include_directories (fluka PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/corsika_modules/fluka>
      )
    
    set_target_properties (
      fluka
      PROPERTIES
      POSITION_INDEPENDENT_CODE 1
      )
    
    install (
      FILES
      fluka.hpp
      DESTINATION include/corsika_modules/fluka
      )
    
    install (
      TARGETS fluka
      EXPORT CORSIKA8PublicTargets
      ARCHIVE DESTINATION lib/corsika
      INCLUDES DESTINATION include/corsika_modules/fluka
      )
    
    # add fluka to corsika8 build
    add_dependencies (CORSIKA8 fluka)
    target_link_libraries (CORSIKA8 INTERFACE fluka)
endif()
