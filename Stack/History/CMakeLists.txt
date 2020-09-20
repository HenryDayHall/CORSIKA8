set (
  HISTORY_HEADERS
  Event.hpp
  HistorySecondaryView.hpp
  HistoryStackExtension.h
  SecondaryParticle.hpp
  )

set (
  HISTORY_NAMESPACE
  corsika/history
  )

add_library (CORSIKAhistory INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAhistory ${HISTORY_NAMESPACE} ${HISTORY_HEADERS})

target_link_libraries (
  CORSIKAhistory
  INTERFACE
  CORSIKAparticles
  CORSIKAgeometry
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
  FILES ${HISTORY_HEADERS} 
  DESTINATION include/${HISTORY_NAMESPACE}
  )

# ----------------
# code unit testing
CORSIKA_ADD_TEST(testHistoryStack)
target_link_libraries (
  testHistoryStack
  CORSIKAhistory
  CORSIKAtesting
  )
CORSIKA_ADD_TEST(testHistoryView)
target_link_libraries (
  testHistoryView
  CORSIKAhistory
  CORSIKAtesting
  )
