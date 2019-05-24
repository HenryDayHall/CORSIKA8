set (
  MODEL_SOURCES
  HadronicElasticModel.cc
  )

set (
  MODEL_HEADERS
  HadronicElasticModel.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/hadronic_elastic_model
  )

add_library (ProcessHadronicElasticModel STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessHadronicElasticModel ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessHadronicElasticModel
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessHadronicElasticModel
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAsetup
  CORSIKAutilities
  )

target_include_directories (
  ProcessHadronicElasticModel 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessHadronicElasticModel
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
# CORSIKA_ADD_TEST (testProcessHadronicElasticModel)
# target_link_libraries (
  # testProcessHadronicElasticModel
  # ProcessHadronicElasticModel
  # CORSIKAsetup
  # CORSIKAgeometry
  # CORSIKAunits
  # CORSIKAthirdparty # for catch2
  # )
