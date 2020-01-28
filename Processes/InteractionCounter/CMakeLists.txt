set (
  MODEL_HEADERS
  InteractionCounter.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/interaction_counter
  )

FIND_PACKAGE( Boost 1.70 REQUIRED )

add_library (ProcessInteractionCounter INTERFACE)
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessInteractionCounter ${MODEL_NAMESPACE} ${MODEL_HEADERS})

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessInteractionCounter
  INTERFACE
  CORSIKAunits
  CORSIKAprocesssequence
  )

target_include_directories (
  ProcessInteractionCounter 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  ${Boost_INCLUDE_DIR}
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# --------------------
# code unit testing
#~ CORSIKA_ADD_TEST(testLuminosityCounter)
#~ target_link_libraries (
  #~ testLuminosityCounter
  #~ ProcessLuminosityCounter
  #~ CORSIKAstackinterface
  #~ CORSIKAtesting
#~ )

