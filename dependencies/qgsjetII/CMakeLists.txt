cmake_minimum_required(VERSION 3.1)

project(libQGSJetII)

set (
  MODEL_SOURCES
  qgsjet-II-04-mod1.f
  )

set (
  MODEL_HEADERS
  qgsjet-II-04.h
  )


enable_language(Fortran)
add_library (QGSJetII SHARED ${MODEL_SOURCES})
set_property(TARGET QGSJetII PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories (
  QGSJetII
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/include>
  )


add_library (QGSJetII_static STATIC ${MODEL_SOURCES})
set_property(TARGET QGSJetII_static PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories (
  QGSJetII_static
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/include>
  )


# add qgsjetII to corsika8 build
add_dependencies (CORSIKA8 QGSJetII_static)
target_link_libraries (CORSIKA8 INTERFACE QGSJetII_static)
