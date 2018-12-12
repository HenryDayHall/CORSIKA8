
add_library (CORSIKAprocesssequence INTERFACE)

#namespace of library->location of header files
set (
  CORSIKAprocesssequence_NAMESPACE
  corsika/process
  )

#header files of this library
set (
  CORSIKAprocesssequence_HEADERS
  BaseProcess.h
  ContinuousProcess.h
  InteractionProcess.h
  DecayProcess.h
  ProcessSequence.h
  ProcessReturn.h
  )

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAprocesssequence ${CORSIKAprocesssequence_NAMESPACE} ${CORSIKAprocesssequence_HEADERS})

#include directive for upstream code
target_include_directories (
  CORSIKAprocesssequence
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  )

#install library
install (
  FILES ${CORSIKAprocesssequence_HEADERS}
  DESTINATION include/${CORSIKAprocesssequence_NAMESPACE}
  )

#-- -- -- -- -- -- -- --
#code unit testing
add_executable (
  testProcessSequence
  testProcessSequence.cc
  )

target_link_libraries (
  testProcessSequence
  CORSIKAsetup
  CORSIKAgeometry
  CORSIKAprocesssequence
  CORSIKAthirdparty # for catch2
  )

add_test (
  NAME testProcessSequence
  COMMAND testProcessSequence
  )
