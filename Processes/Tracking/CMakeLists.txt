set (
  MODEL_HEADERS
  Intersect.hpp
  )

set (
  MODEL_NAMESPACE
  corsika/process/tracking
  )

add_library (ProcessTrackingIntersects INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessTrackingIntersects ${MODEL_NAMESPACE} ${MODEL_HEADERS})

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessTrackingIntersects
  INTERFACE
  CORSIKAsetup
  CORSIKAutilities
  CORSIKAenvironment
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAlogging
  )

target_include_directories (
  ProcessTrackingIntersects
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

