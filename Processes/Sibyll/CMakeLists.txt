add_custom_command (
  OUTPUT  ${PROJECT_BINARY_DIR}/Processes/Sibyll/Generated.inc
  COMMAND ${PROJECT_SOURCE_DIR}/Processes/Sibyll/code_generator.py 
          ${PROJECT_BINARY_DIR}/Framework/Particles/pythia_db.pkl
          ${PROJECT_SOURCE_DIR}/Processes/Sibyll/sibyll_codes.dat
  DEPENDS code_generator.py
          sibyll_codes.dat          
          ${PROJECT_BINARY_DIR}/Framework/Particles/pythia_db.pkl
  WORKING_DIRECTORY
          ${PROJECT_BINARY_DIR}/Processes/Sibyll/Particles/
  COMMENT "Generate conversion tables for particle codes SIBYLL <-> CORSIKA"
  VERBATIM
  )
  
set (
  MODEL_SOURCES
  ParticleConversion.cc
  )

set (
  MODEL_HEADERS
  ParticleConversion.h
  ${PROJECT_BINARY_DIR}/Processes/Sibyll/Generated.inc
  )

set (
  MODEL_NAMESPACE
  corsika/process/sibyll
  )

add_library (ProcessSibyll STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessSibyll ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessSibyll
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessSibyll
  CORSIKAunits
  )

target_include_directories (
  ProcessSibyll 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessSibyll
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
add_executable (testSibyll
  testSibyll.cc
  ${MODEL_HEADERS}
  )

target_link_libraries (
  testSibyll
  CORSIKAgeometry
  CORSIKAunits
  CORSIKAthirdparty # for catch2
  )

add_test (NAME testSibyll COMMAND testSibyll)

