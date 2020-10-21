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
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  cnpy
  PUBLIC
  ZLIB::ZLIB
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
  )

