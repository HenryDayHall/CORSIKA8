#cmake_minimum_required(VERSION 3.1)
#project(Sibyll)

set (
  MODEL_SOURCES
  sibyll2.3d.cpp
  sibyll2.3d.f
  nuclib.f
  signuc.f
  gasdev.f
  #rndm_dbl.f
  )

set (
  MODEL_HEADERS
  sibyll2.3d.hpp
  nuclib.hpp
  )

enable_language(Fortran)
add_library (Sibyll SHARED ${MODEL_SOURCES})
set_property(TARGET Sibyll PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library (Sibyll_static STATIC ${MODEL_SOURCES})
set_property(TARGET Sibyll_static PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories (
  Sibyll
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/include>
  )

target_include_directories (
  Sibyll_static
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/include>
  )

target_link_libraries (
  Sibyll_static
  PUBLIC
  gfortran
  )

add_dependencies (CORSIKA8 Sibyll_static)
