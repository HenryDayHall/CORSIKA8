set (
  MODEL_SOURCES
  sibyll2.3d.cpp
  sibyll2.3d.f
  nuclib.f
  signuc.f
  gasdev.f
  )

set (
  MODEL_HEADERS
  sibyll2.3d.hpp
  nuclib.hpp
  )

enable_language (Fortran)
add_library (Sibyll_static STATIC ${MODEL_SOURCES})

set_target_properties (
  Sibyll_static
  PROPERTIES
  POSITION_INDEPENDENT_CODE 1
  )

target_include_directories (
  Sibyll_static
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/corsika_modules/sibyll>
  )

target_link_libraries (
  Sibyll_static
  PUBLIC
  gfortran
  )

install (
  FILES
  ${MODEL_HEADERS}
  DESTINATION include/corsika_modules/sibyll
  )

install (
  TARGETS Sibyll_static
  EXPORT CORSIKA8PublicTargets
  ARCHIVE DESTINATION lib/corsika
  )

# add sibyll to corsika8 build
add_dependencies (CORSIKA8 Sibyll_static)
target_link_libraries (CORSIKA8 INTERFACE Sibyll_static)
