set (
  MODEL_HEADERS
  Tracking.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/tracking_leapfrog_curved
  )

add_library (ProcessTrackingLeapFrogCurved INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessTrackingLeapFrogCurved ${MODEL_NAMESPACE} ${MODEL_HEADERS})

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessTrackingLeapFrogCurved
  INTERFACE
  ProcessTrackingIntersects
  CORSIKAsetup
  CORSIKAutilities
  CORSIKAenvironment
  CORSIKAunits
  CORSIKAenvironment
  CORSIKAgeometry
  CORSIKAlogging
  )

target_include_directories (
  ProcessTrackingLeapFrogCurved 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# #-- -- -- -- -- -- -- -- -- --
# #code unit testing
CORSIKA_ADD_TEST (testTrackingLeapFrogCurved)
target_link_libraries (
   testTrackingLeapFrogCurved
   ProcessTrackingLeapFrogCurved
   CORSIKAtesting
)
