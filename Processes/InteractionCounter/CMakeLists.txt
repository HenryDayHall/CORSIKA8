set (
  MODEL_HEADERS
  InteractionCounter.hpp
  InteractionHistogram.hpp
  )

set (
  MODEL_SOURCES
  InteractionHistogram.cc
)

set (
  MODEL_NAMESPACE
  corsika/process/interaction_counter
  )

add_library (ProcessInteractionCounter STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessInteractionCounter ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessInteractionCounter
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessInteractionCounter
  CORSIKAunits
  CORSIKAutilities
  CORSIKAprocesssequence
  CORSIKAthirdparty
  C8::ext::boost
  )

target_include_directories (
  ProcessInteractionCounter 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (FILES ${MODEL_HEADERS} DESTINATION include/${MODEL_NAMESPACE})

# --------------------
# code unit testing
CORSIKA_ADD_TEST(testInteractionCounter SOURCES
  testInteractionCounter.cc
  ${MODEL_HEADERS}
)
#
target_compile_definitions (
  testInteractionCounter
  PRIVATE
  REFDATADIR="${CMAKE_CURRENT_SOURCE_DIR}"
  )
#
target_link_libraries (
  testInteractionCounter
  ProcessInteractionCounter
  CORSIKAsetup
  CORSIKAunits
  CORSIKAenvironment
  CORSIKAtesting
  )
