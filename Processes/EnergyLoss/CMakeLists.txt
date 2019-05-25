set (
  MODEL_SOURCES
  EnergyLoss.cc
  )

set (
  MODEL_HEADERS
  EnergyLoss.h
  )

set (
  MODEL_NAMESPACE
  corsika/process/energy_loss
  )

add_library (ProcessEnergyLoss STATIC ${MODEL_SOURCES})
CORSIKA_COPY_HEADERS_TO_NAMESPACE (ProcessEnergyLoss ${MODEL_NAMESPACE} ${MODEL_HEADERS})

set_target_properties (
  ProcessEnergyLoss
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
#  PUBLIC_HEADER "${MODEL_HEADERS}"
  )

# target dependencies on other libraries (also the header onlys)
target_link_libraries (
  ProcessEnergyLoss
  CORSIKAunits
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAsetup
  )

target_include_directories (
  ProcessEnergyLoss 
  INTERFACE 
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/include>
  )

install (
  TARGETS ProcessEnergyLoss
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
#  PUBLIC_HEADER DESTINATION include/${MODEL_NAMESPACE}
  )


# --------------------
# code unit testing
#CORSIKA_ADD_TEST (testNullModel testNullModel.cc)
#target_link_libraries (
#  testNullModel  ProcessNullModel
#  CORSIKAsetup
#  CORSIKAgeometry
#  CORSIKAunits
#  CORSIKAthirdparty # for catch2
#  )

