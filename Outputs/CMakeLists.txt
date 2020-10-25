 set (
  MODEL_HEADERS
  BaseOutput.h
  OutputManager.h
  ParquetStreamer.h
  ObservationPlaneWriterParquet.h
  )

set (
  MODEL_SOURCES
  OutputManager.cc
  )

set (
  MODEL_NAMESPACE
  corsika/output/
  )

add_library (CORSIKAoutput STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAoutput ${MODEL_NAMESPACE} ${MODEL_HEADERS})

#target dependencies on other libraries(also the header onlys)
target_link_libraries (
  CORSIKAoutput
  CORSIKAgeometry
  CORSIKAlogging
  CORSIKAthirdparty
  CORSIKAprocesssequence
  )

target_include_directories (
  CORSIKAoutput
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})
