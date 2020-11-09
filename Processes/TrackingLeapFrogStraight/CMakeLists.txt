set (
  MODEL_HEADERS
  Tracking.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/tracking_leapfrag_straight
  )

add_library (ProcessTrackingLeapFrogStraight INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessTrackingLeapFrogStraight ${MODEL_NAMESPACE} ${MODEL_HEADERS})

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessTrackingLeapFrogStraight
  INTERFACE
  CORSIKAsetup
  CORSIKAutilities
  CORSIKAenvironment
  CORSIKAunits
  CORSIKAenvironment
  CORSIKAgeometry
  CORSIKAlogging
  )

target_include_directories (
  ProcessTrackingLeapFrogStraight 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# #-- -- -- -- -- -- -- -- -- --
# #code unit testing
CORSIKA_ADD_TEST (testTrackingLeapFrogStraight)
target_link_libraries (
   testTrackingLeapFrogStraight
   ProcessTrackingLeapFrogStraight
   CORSIKAtesting
)
