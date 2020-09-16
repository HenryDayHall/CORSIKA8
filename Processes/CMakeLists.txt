# general
add_subdirectory (AnalyticProcessors)
add_subdirectory (ExampleProcessors)

add_subdirectory (NullModel)
# tracking
add_subdirectory (TrackingLine)
# hadron interaction models
add_subdirectory (Sibyll)
add_subdirectory (QGSJetII)
if (Pythia8_FOUND)
  add_subdirectory (Pythia)
endif (Pythia8_FOUND)
if (CONEX_FOUND)
  add_subdirectory (CONEXSourceCut)
endif (CONEX_FOUND)
add_subdirectory (HadronicElasticModel)
add_subdirectory (UrQMD)
add_subdirectory (Proposal)

# continuous physics
add_subdirectory (EnergyLoss)
add_subdirectory (LongitudinalProfile)
add_subdirectory (TrackWriter)
add_subdirectory (ObservationPlane)
# stack processes
add_subdirectory (StackInspector)
# secondaries process
# cuts, thinning, etc.
add_subdirectory (ParticleCut)
add_subdirectory (OnShellCheck)
# meta-processes
add_subdirectory (InteractionCounter)
add_subdirectory (SwitchProcess)

<<<<<<< HEAD
=======
add_subdirectory (DevTools)
>>>>>>> Added first implementations of timeing analytics for processor

##########################################
# add_custom_target(CORSIKAprocesses)
add_library (CORSIKAprocesses INTERFACE)
add_dependencies (CORSIKAprocesses ProcessNullModel)
add_dependencies (CORSIKAprocesses ProcessSibyll)
add_dependencies (CORSIKAprocesses ProcessProposal)
if (Pythia8_FOUND)
  add_dependencies (CORSIKAprocesses ProcessPythia8)
endif (Pythia8_FOUND)
add_dependencies (CORSIKAprocesses ProcessStackInspector)
add_dependencies (CORSIKAprocesses ProcessTrackingLine)
add_dependencies (CORSIKAprocesses ProcessEnergyLoss)
add_dependencies (CORSIKAprocesses ProcessUrQMD)
add_dependencies (CORSIKAprocesses ProcessParticleCut)
add_dependencies (CORSIKAprocesses ProcessOnShellCheck)
