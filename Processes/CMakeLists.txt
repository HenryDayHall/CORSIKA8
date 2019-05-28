# general
add_subdirectory (NullModel)
# tracking
add_subdirectory (TrackingLine) 
# hadron interaction models
add_subdirectory (Sibyll)
if (PYTHIA8_FOUND)
  add_subdirectory (Pythia)
endif (PYTHIA8_FOUND)
add_subdirectory (HadronicElasticModel)
add_subdirectory (UrQMD)
add_subdirectory (SwitchProcess)

# continuous physics
add_subdirectory (EnergyLoss)
add_subdirectory (TrackWriter)
add_subdirectory (ObservationPlane)
# stack processes
add_subdirectory (StackInspector)
# secondaries process
# cuts, thinning, etc.
add_subdirectory (ParticleCut)

##########################################
# add_custom_target(CORSIKAprocesses)
add_library (CORSIKAprocesses INTERFACE)
add_dependencies(CORSIKAprocesses ProcessNullModel)
add_dependencies(CORSIKAprocesses ProcessSibyll)
if (PYTHIA8_FOUND)
  add_dependencies(CORSIKAprocesses ProcessPythia)
endif (PYTHIA8_FOUND)
add_dependencies(CORSIKAprocesses ProcessStackInspector)
add_dependencies(CORSIKAprocesses ProcessTrackingLine)
add_dependencies(CORSIKAprocesses ProcessEnergyLoss)
add_dependencies(CORSIKAprocesses ProcessUrQMD)
add_dependencies(CORSIKAprocesses ProcessParticleCut)
