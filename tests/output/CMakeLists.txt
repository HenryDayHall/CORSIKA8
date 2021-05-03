set (test_output_sources
  TestMain.cpp
  testOutputManager.cpp
  testDummyOutputManager.cpp
  testParquetStreamer.cpp
  testWriterObservationPlane.cpp
  testWriterTracks.cpp
  )

CORSIKA_ADD_TEST (testOutput SOURCES ${test_output_sources})

target_compile_definitions (
  testOutput
  PRIVATE
  REFDATADIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
