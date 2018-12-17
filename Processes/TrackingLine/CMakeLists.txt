
set (
  MODEL_HEADERS
  TrackingLine.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/tracking_line
  )

add_library (ProcessTrackingLine INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessTrackingLine ${MODEL_NAMESPACE} ${MODEL_HEADERS})


target_include_directories (
  ProcessTrackingLine 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# #-- -- -- -- -- -- -- -- -- --
# #code unit testing
add_executable (testTrackingLine testTrackingLine.cc)

target_link_libraries (
   testTrackingLine
   ProcessTrackingLine
   CORSIKAsetup
   CORSIKAutilities
   CORSIKAenvironment
   CORSIKAunits
   CORSIKAenvironment
   CORSIKAgeometry
   CORSIKAthirdparty # for catch2
)

add_test (NAME testTrackingLine COMMAND testTrackingLine)
