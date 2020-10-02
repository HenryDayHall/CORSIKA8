set (
  HISTORY_HEADERS
  Event.hpp
  HistorySecondaryProducer.hpp
  HistoryObservationPlane.hpp
  HistoryStackExtension.hpp
  SecondaryParticle.hpp
  )

set (
  HISTORY_NAMESPACE
  corsika/stack/history
  )

set (
  HISTORY_SOURCES
  HistoryObservationPlane.cpp
)

#add_library (CORSIKAhistory INTERFACE)
add_library (CORSIKAhistory STATIC ${HISTORY_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAhistory ${HISTORY_NAMESPACE} ${HISTORY_HEADERS})

target_link_libraries (
  CORSIKAhistory
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAprocesssequence # for HistoryObservationPlane
  CORSIKAsetup # for HistoryObservationPlane
  CORSIKAlogging
  SuperStupidStack
  NuclearStackExtension # for testHistoryView.cc
  C8::ext::boost # for HistoryObservationPlane
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
  DummyStack
  )
CORSIKA_ADD_TEST(testHistoryView)
target_link_libraries (
  testHistoryView
  CORSIKAhistory
  CORSIKAtesting
  )
