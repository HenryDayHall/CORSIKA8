# namespace of library -> location of header files
set (
  CORSIKAcascade_NAMESPACE
  corsika/cascade
  )

# header files of this library
set (
  CORSIKAcascade_HEADERS
  Cascade.h
  testCascade.h
  )

add_library (CORSIKAcascade INTERFACE)

CORSIKA_COPY_HEADERS_TO_NAMESPACE (CORSIKAcascade ${CORSIKAcascade_NAMESPACE} ${CORSIKAcascade_HEADERS})

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
CORSIKA_ADD_TEST(testCascade)
target_link_libraries (
  testCascade
  #  CORSIKAutls
  CORSIKArandom
  ProcessSibyll
  CORSIKAcascade
  ProcessStackInspector
  ProcessTrackingLine
  ProcessNullModel
  CORSIKAstackinterface
  CORSIKAprocesses
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAtesting
  )
