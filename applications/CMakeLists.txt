add_executable (c8_air_shower c8_air_shower.cpp)
target_link_libraries (c8_air_shower PRIVATE CORSIKA8)

set_target_properties(c8_air_shower PROPERTIES
  # use relative path that also works when cmake-install is invoked with --prefix
  INSTALL_RPATH "$ORIGIN/../lib/corsika"
)

if(WITH_FLUKA)
  message("compiling c8_air_shower.cpp with FLUKA")
  target_compile_definitions(c8_air_shower PRIVATE WITH_FLUKA)
else()
  message("compiling c8_air_shower.cpp with UrQMD")
endif()

install (
  TARGETS c8_air_shower DESTINATION bin
)
