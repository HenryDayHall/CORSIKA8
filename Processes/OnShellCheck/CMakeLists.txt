set (
  MODEL_SOURCES
  OnShellCheck.cc
)

set (
  MODEL_HEADERS
  OnShellCheck.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/on_shell_check
  )

add_library (ProcessOnShellCheck STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessOnShellCheck ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessOnShellCheck
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessOnShellCheck
  CORSIKAunits
  CORSIKAparticles
  CORSIKAprocesssequence
  CORSIKAsetup
  )

target_include_directories (
  ProcessOnShellCheck 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessOnShellCheck
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )

# --------------------
# code unit testing
CORSIKA_ADD_TEST(testOnShellCheck SOURCES
  testOnShellCheck.cc
  ${MODEL_HEADERS}
)

target_link_libraries (
  testOnShellCheck
  ProcessOnShellCheck
  CORSIKAunits
  CORSIKAstackinterface
  CORSIKAprocesssequence
  CORSIKAsetup
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAtesting
  )
