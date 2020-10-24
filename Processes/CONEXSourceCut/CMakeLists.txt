set (
  MODEL_SOURCES
  CONEXSourceCut.cc
)

set (
  MODEL_HEADERS
  CONEXSourceCut.h
  CONEX_f.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/conex_source_cut
  )

add_library (ProcessCONEXSourceCut STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessCONEXSourceCut ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessCONEXSourceCut
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessCONEXSourceCut
  CORSIKAunits
  CORSIKAparticles
  CORSIKAprocesssequence
  CORSIKAsetup
  C8::ext::conex
  ProcessSibyll
  ProcessUrQMD
  )

target_include_directories (
  ProcessCONEXSourceCut 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessCONEXSourceCut
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

# --------------------
# code unit testing
CORSIKA_ADD_TEST(testCONEXSourceCut SOURCES
  testCONEXSourceCut.cc
  ${MODEL_HEADERS}
)
# 
target_compile_definitions (
  testCONEXSourceCut
  PRIVATE
  REFDATADIR="${CMAKE_CURRENT_SOURCE_DIR}"
  )
#
target_link_libraries (
  testCONEXSourceCut
  ProcessCONEXSourceCut
  CORSIKAunits
  CORSIKAstackinterface
  CORSIKAprocesssequence
  CORSIKAsetup
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAtesting
)
