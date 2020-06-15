set (
  MODEL_SOURCES
  LongitudinalProfile.cc
  )

set (
  MODEL_HEADERS
  LongitudinalProfile.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/longitudinal_profile
  )

add_library (ProcessLongitudinalProfile STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessLongitudinalProfile ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessLongitudinalProfile
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessLongitudinalProfile
  CORSIKAenvironment
  CORSIKAunits
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAsetup
  )

target_include_directories (
  ProcessLongitudinalProfile 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessLongitudinalProfile
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

