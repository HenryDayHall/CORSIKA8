set (
  MODEL_HEADERS
  SwitchProcess.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/switch_process
  )

add_library (ProcessSwitch INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessSwitch ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessStackInspector
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessSwitch
  INTERFACE
  CORSIKAunits
  CORSIKAprocesssequence
  )

target_include_directories (
  ProcessSwitch 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# --------------------
# code unit testing
#add_executable (testStackInspector testSwitchProcess.cc)

#target_link_libraries (
#  testSwitchProcess
#  ProcessSwitch
#  CORSIKAthirdparty # for catch2
#  )
#CORSIKA_ADD_TEST(testStackInspector)
