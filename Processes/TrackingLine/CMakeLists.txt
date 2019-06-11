set (
  MODEL_HEADERS
  TrackingLine.h
  )

set (
  MODEL_SOURCES
  TrackingLine.cc
  )

set (
  MODEL_NAMESPACE
  corsika/process/tracking_line
  )

add_library (ProcessTrackingLine STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessTrackingLine ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessTrackingLine
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessTrackingLine
  CORSIKAsetup
  CORSIKAutilities
  CORSIKAenvironment
  CORSIKAunits
  CORSIKAenvironment
  CORSIKAgeometry
  )

target_include_directories (
  ProcessTrackingLine 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# #-- -- -- -- -- -- -- -- -- --
# #code unit testing
CORSIKA_ADD_TEST (testTrackingLine)
target_link_libraries (
   testTrackingLine
   ProcessTrackingLine
   CORSIKAtesting
)
