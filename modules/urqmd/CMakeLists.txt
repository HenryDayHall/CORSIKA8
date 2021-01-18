cmake_minimum_required (VERSION 3.1)
project (libUrQMD)

set (
  MODEL_SOURCES
  urqmd.cpp
  urqmdInterface.F
  addpart.f
  angdis.f
  anndec.f
  blockres.f
  boxprg.f
  cascinit.f
  coload.f
  dectim.f
  delpart.f
  detbal.f
  dwidth.f
  error.f
  getmass.f
  getspin.f
  init.f
  iso.f
  ityp2pdg.f
  jdecay2.f
  make22.f
  numrec.f
  output.f
  paulibl.f
  proppot.f
  saveinfo.f
  scatter.f
  siglookup.f
  string.f
  tabinit.f
  urqmd.f
  whichres.f
)

set (
  MODEL_HEADERS
  urqmd.hpp
  )

enable_language (Fortran)

add_library (UrQMD SHARED ${MODEL_SOURCES})
target_include_directories (UrQMD PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/modules/urqmd>
  )
set_target_properties (UrQMD PROPERTIES POSITION_INDEPENDENT_CODE 1)

add_library (UrQMD_static STATIC ${MODEL_SOURCES})
target_include_directories (UrQMD_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/modules/urqmd>
  )
set_target_properties (UrQMD_static PROPERTIES POSITION_INDEPENDENT_CODE 1)

install (
  TARGETS UrQMD
  EXPORT CORSIKA8
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION include 
  )

install (
  TARGETS UrQMD_static
  EXPORT CORSIKA8
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION include 
  )

# add UrQMD to CORSIKA8 build
add_dependencies (CORSIKA8 UrQMD_static)
target_link_libraries (CORSIKA8 INTERFACE UrQMD_static)
