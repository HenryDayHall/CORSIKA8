
set (
  CORSIKArandom_SOURCES
  RNGManager.cc
  )

set (
  CORSIKArandom_HEADERS
  RNGManager.h
  UniformRealDistribution.h
  )

set (
  CORSIKArandom_NAMESPACE
  corsika/random
  )

add_library (CORSIKArandom STATIC ${CORSIKArandom_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKArandom ${CORSIKArandom_NAMESPACE} ${CORSIKArandom_HEADERS})

target_include_directories (
  CORSIKArandom
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  )

# target dependencies on other libraries (also the header onlys)
# none

install (
  TARGETS CORSIKArandom
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${CORSIKArandom_NAMESPACE}
  )


# --------------------
# code unit testing
add_executable (testRandom testRandom.cc)

target_link_libraries (
  testRandom
  CORSIKArandom
  CORSIKAthirdparty # for catch2
  )
CORSIKA_ADD_TEST(testRandom)
