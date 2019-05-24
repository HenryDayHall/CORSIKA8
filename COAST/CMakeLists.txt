set (
  COAST_HEADERS
  COASTProcess.h
  COASTStack.h
  ParticleConversion.h
  )

set (
  COAST_SOURCES
  COASTUserLib.cc
  COASTProcess.cc
  ParticleConversion.cc
  )

set (
  COAST_NAMESPACE
  corsika/coast
  )

add_library (COAST SHARED ${COAST_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (COAST ${COAST_NAMESPACE} ${COAST_HEADERS})

set_target_properties (
  COAST
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

target_link_libraries (
  COAST
  PUBLIC
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAsetup
  # SuperStupidStack
  )

target_include_directories (
  COAST
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

target_include_directories (
  COAST
  SYSTEM
  PUBLIC
  $ENV{COAST_DIR}/include
  )

install (
  TARGETS COAST
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${COAST_NAMESPACE}
  )

#install (
#  FILES ${COAST_HEADERS} 
#  DESTINATION include/${COAST_NAMESPACE}
#  )
