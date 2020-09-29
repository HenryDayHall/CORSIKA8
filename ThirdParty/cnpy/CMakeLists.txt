set (
  CNPY_SOURCES
  cnpy.cpp
  )

set (
  CNPY_HEADERS
  cnpy.hpp
  )

set (
  CNPY_NAMESPACE
  corsika/third_party/cnpy
  )

add_library (cnpy STATIC ${CNPY_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (cnpy ${CNPY_NAMESPACE} ${CNPY_HEADERS})

set_target_properties (
  cnpy
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

find_package(ZLIB)

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  cnpy
  ${ZLIB_LIBRARIES}
  )

target_include_directories (
  cnpy 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS cnpy
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

