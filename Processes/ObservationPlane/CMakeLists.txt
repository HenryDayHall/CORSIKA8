set (
  MODEL_HEADERS
  ObservationPlane.h
  )

set (
  MODEL_SOURCES
  ObservationPlane.cc
  )

set (
  MODEL_NAMESPACE
  corsika/process/observation_plane
  )

add_library (ProcessObservationPlane STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessObservationPlane ${MODEL_NAMESPACE} ${MODEL_HEADERS})

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessObservationPlane
  CORSIKAgeometry
  CORSIKAprocesssequence
  )

target_include_directories (
  ProcessObservationPlane 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# --------------------
# code unit testing
CORSIKA_ADD_TEST(testObservationPlane)
target_link_libraries (
  testObservationPlane
  ProcessObservationPlane
  CORSIKAstackinterface
  CORSIKAthirdparty # for catch2
)

