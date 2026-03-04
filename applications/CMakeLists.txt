add_executable (c8_air_shower c8_air_shower.cpp)
add_executable (c8_air_shower_with_history c8_air_shower_with_history.cpp)
target_link_libraries (c8_air_shower PRIVATE CORSIKA8)
target_link_libraries (c8_air_shower_with_history PRIVATE CORSIKA8)

set_target_properties(c8_air_shower PROPERTIES
  # use relative path that also works when cmake-install is invoked with --prefix
  INSTALL_RPATH "$ORIGIN/../lib/corsika"
)
set_target_properties(c8_air_shower_with_history PROPERTIES
  # use relative path that also works when cmake-install is invoked with --prefix
  INSTALL_RPATH "$ORIGIN/../lib/corsika"
)


if(WITH_FLUKA)
  message("compiling c8_air_shower.cpp with FLUKA")
  target_compile_definitions(c8_air_shower PRIVATE WITH_FLUKA)
  target_compile_definitions(c8_air_shower_with_history PRIVATE WITH_FLUKA)
else()
  message("compiling c8_air_shower.cpp with UrQMD")
endif()

install (
  TARGETS c8_air_shower c8_air_shower_with_history DESTINATION bin
)
