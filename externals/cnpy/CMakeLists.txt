set (
  CNPY_SOURCES
  cnpy.cpp
  )

set (
  CNPY_HEADERS
  cnpy.hpp
  )

add_library (cnpy STATIC ${CNPY_SOURCES})

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  cnpy
  PUBLIC
  CONAN_PKG::zlib
  )

target_include_directories (
  cnpy 
  INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/externals/cnpy>
  )

install (
  TARGETS cnpy
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

