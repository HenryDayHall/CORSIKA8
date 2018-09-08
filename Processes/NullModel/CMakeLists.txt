
set (
  MODEL_SOURCES
  NullModel.cc
  )

set (
  MODEL_HEADERS
  NullModel.h
  )

set (
  MODEL_NAMESPACE
  process/null_model
  )

add_library (ProcessNullModel STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessNullModel ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessNullModel
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessNullModel
  CORSIKAunits
  )

target_include_directories (
  ProcessNullModel 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessNullModel
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
add_executable (testNullModel testNullModel.cc)

target_link_libraries (
  testNullModel
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAthirdparty # for catch2
  )

add_test (NAME testNullModel COMMAND testNullModel)

