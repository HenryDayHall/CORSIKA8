set (CMAKE_BUILT_TYPE Debug)
#set (CMAKE_Fortran_FLAGS "-std=legacy -Wfunction-elimination -O0")

set (
  MODEL_SOURCES
  epos.cpp
  epos-bas-lhc.f  epos-con-lhc.f eposm.f epos_j.f 
  epos-dky-lhc.f  epos-dro-lhc.f epos-ems-lhc.f  
  epos-fra-lhc.f  epos-ico-lhc.f epos-hnb-lhc.f 
  epos-ids-lhc.f  epos-int-lhc.f epos-jps-lhc.f  
  epos-omg-lhc.f  epos-par-lhc.f epos-sem-lhc.f 
  epos-rsh-lhc.f  epos-qsh-lhc.f epos-tim-lhc.f 
  epos-uti-lhc.f  epos-xan-lhc.f epos-xpr-lhc.f  
  )

set (
  MODEL_HEADERS
  epos.hpp
  )

enable_language (Fortran)

add_library (EPOS_static STATIC ${MODEL_SOURCES})
target_include_directories (EPOS_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/corsika_modules/epos>
  )
set_target_properties (
  EPOS_static
  PROPERTIES
  POSITION_INDEPENDENT_CODE 1
  )
target_compile_options (EPOS_static BEFORE PRIVATE -O0)

install (
  FILES
  ${MODEL_HEADERS}
  DESTINATION include/corsika_modules/epos
  )

install (
  TARGETS EPOS_static
  EXPORT CORSIKA8PublicTargets
  ARCHIVE DESTINATION lib/corsika
  INCLUDES DESTINATION include/corsika_modules/epos
  )

# add EPOS to CORSIKA8 build
add_dependencies (CORSIKA8 EPOS_static)
target_link_libraries (CORSIKA8 INTERFACE EPOS_static)
