set (
  MODEL_SOURCES
  TrackWriter.cc
  )

set (
  MODEL_HEADERS
  TrackWriter.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/track_writer
  )

add_library (ProcessTrackWriter STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessTrackWriter ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessTrackWriter
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessTrackWriter
  CORSIKAunits
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAsetup
  )

target_include_directories (
  ProcessTrackWriter 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessTrackWriter
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
# CORSIKA_ADD_TEST(testNullModel)
#target_link_libraries (
#  testNullModel  ProcessNullModel
#  CORSIKAsetup
#  CORSIKAgeometry
#  CORSIKAunits
#  CORSIKAthirdparty # for catch2
#  )

