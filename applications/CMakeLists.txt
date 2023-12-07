SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

if(WITH_FLUKA)
  message("compiling c8_air_shower.cpp with FLUKA")
else()
  message("compiling c8_air_shower.cpp with UrQMD")
endif()
add_executable (c8_air_shower c8_air_shower.cpp)
target_link_libraries (c8_air_shower CORSIKA8)

install (
  TARGETS c8_air_shower DESTINATION bin
)
