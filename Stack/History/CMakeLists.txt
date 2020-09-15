set (
  SETUP_HEADERS
  HistoryStackExtension.h
  )

set (
  SETUP_NAMESPACE
  corsika/history
  )

add_library (CORSIKAhistory INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAhistory ${SETUP_NAMESPACE} ${SETUP_HEADERS})

target_link_libraries (
  CORSIKAhistory
  INTERFACE
  SuperStupidStack
  NuclearStackExtension
  )

target_include_directories (
  CORSIKAhistory
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  FILES ${SETUP_HEADERS} 
  DESTINATION include/${SETUP_NAMESPACE}
  )

# ----------------
# code unit testing
CORSIKA_ADD_TEST(testHistory)
target_link_libraries (
  testHistory
  SuperStupidStack
  CORSIKAhistory
  CORSIKAparticles
  CORSIKAunits
  CORSIKAtesting
  )
