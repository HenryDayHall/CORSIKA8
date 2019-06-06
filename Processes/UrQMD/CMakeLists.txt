set (
  MODEL_SOURCES
  UrQMD.cc
  urqmdInterface.F
  addpart.f
  angdis.f
  anndec.f
  blockres.f
  boxprg.f
  cascinit.f
  coload.f
  dectim.f
  delpart.f
  detbal.f
  dwidth.f
  error.f
  getmass.f
  getspin.f
  init.f
  iso.f
  ityp2pdg.f
  jdecay2.f
  make22.f
  numrec.f
  output.f
  paulibl.f
  proppot.f
  saveinfo.f
  scatter.f
  siglookup.f
  string.f
  tabinit.f
  urqmd.f
  whichres.f
)

set (
  MODEL_HEADERS
  UrQMD.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/urqmd
  )

add_library (ProcessUrQMD STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessUrQMD ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessUrQMD
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessUrQMD
  CORSIKAprocesssequence
  CORSIKAparticles
  CORSIKAunits
  CORSIKAgeometry
  CORSIKArandom
  CORSIKAsetup
  )

target_include_directories (
  ProcessUrQMD 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessUrQMD
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
CORSIKA_ADD_TEST(testUrQMD SOURCES testUrQMD.cc ${MODEL_HEADERS})
target_link_libraries (
  testUrQMD
  ProcessUrQMD
  CORSIKAsetup
  CORSIKArandom
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAtesting
  )
