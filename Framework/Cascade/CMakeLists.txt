
# namespace of library -> location of header files
set (
  CORSIKAcascade_NAMESPACE
  corsika/cascade
  )

# header files of this library
set (
  CORSIKAcascade_HEADERS
  Cascade.h
  sibyll2.3c.h
  )

#set (
#  CORSIKAcascade_SOURCES
#  Cascade.cc
#  )

set (
  CORSIKAsibyll_NAMESPACE
  corsika/cascade
  )

set (
  CORSIKAsibyll_SOURCES
  sibyll2.3c.f
  rndm_dbl.f
  )
add_library (CORSIKAsibyll STATIC ${CORSIKAsibyll_SOURCES})

#add_library (CORSIKAcascade STATIC ${CORSIKAcascade_SOURCES})
add_library (CORSIKAcascade INTERFACE)



CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAcascade ${CORSIKAcascade_NAMESPACE} ${CORSIKAcascade_HEADERS})

#target_link_libraries (
#  CORSIKAcascade
#  CORSIKAparticles
#  CORSIKAunits
#  CORSIKAthirdparty # for catch2
#  )

# include directive for upstream code
target_include_directories (
  CORSIKAcascade
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  )

# install library
install (
  FILES ${CORSIKAcascade_HEADERS}
  DESTINATION include/${CORSIKAcascade_NAMESPACE}
  )

# ----------------
# code unit testing
add_executable (
  testCascade
  testCascade.cc
  )

target_link_libraries (
  testCascade
  #  CORSIKAutls
  CORSIKArandom
  CORSIKAsibyll
  CORSIKAcascade
  ProcessStackInspector
  CORSIKAprocesses
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAthirdparty # for catch2
  )

add_test (
  NAME testCascade
  COMMAND testCascade
  )
