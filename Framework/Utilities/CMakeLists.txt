#
# cfenv feature test - select implementation to use
#
try_compile (HAS_FEENABLEEXCEPT "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/try_feenableexcept.cc")
if (HAS_FEENABLEEXCEPT)
  set (CORSIKA_FENV "CorsikaFenvDefault.cc")
  set_property(DIRECTORY ${CMAKE_HOME_DIRECTORY} APPEND PROPERTY COMPILE_DEFINITIONS "HAS_FEENABLEEXCEPT")
else ()
  if (APPLE)
    set (CORSIKA_FENV "CorsikaFenvOSX.cc")
  else()
    set (CORSIKA_FENV "CorsikaFenvFallback.cc")
  endif()
endif ()


#
# library setup
#
set (
  UTILITIES_SOURCES  
  COMBoost.cc
  ${CORSIKA_FENV})

set (
  UTILITIES_HEADERS
  COMBoost.h
  Bit.h
  Singleton.h
  sgn.h
  CorsikaFenv.h
  MetaProgramming.h
  )

set (
  UTILITIES_NAMESPACE
  corsika/utl
  )

add_library (CORSIKAutilities STATIC ${UTILITIES_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAutilities ${UTILITIES_NAMESPACE} ${UTILITIES_HEADERS})

set_target_properties (
  CORSIKAutilities
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  PUBLIC_HEADER "${UTILITIES_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  CORSIKAutilities
  CORSIKAgeometry
  CORSIKAunits
  )

target_include_directories (
  CORSIKAutilities
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS CORSIKAutilities
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${UTILITIES_NAMESPACE}
  )


# --------------------
# code unit testing
CORSIKA_ADD_TEST(testCOMBoost)
target_link_libraries (
  testCOMBoost
  CORSIKAutilities
  CORSIKAtesting
  )

CORSIKA_ADD_TEST(testCorsikaFenv)
target_link_libraries (
  testCorsikaFenv
  CORSIKAutilities
  CORSIKAtesting
)
