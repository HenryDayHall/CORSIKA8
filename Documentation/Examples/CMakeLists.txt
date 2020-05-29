
CORSIKA_ADD_EXAMPLE (helix_example)
target_link_libraries (helix_example CORSIKAgeometry CORSIKAunits)
install (TARGETS helix_example DESTINATION share/examples)

CORSIKA_ADD_EXAMPLE (particle_list_example)
target_link_libraries (particle_list_example CORSIKAparticles CORSIKAunits CORSIKAprocesses ProcessSibyll ProcessQGSJetII)
install (TARGETS particle_list_example DESTINATION share/examples)

CORSIKA_ADD_EXAMPLE (geometry_example)
target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)
install (TARGETS geometry_example DESTINATION share/examples)

CORSIKA_ADD_EXAMPLE (logger_example)
target_link_libraries (logger_example CORSIKAunits CORSIKAlogging)
install (TARGETS logger_example DESTINATION share/examples)

CORSIKA_ADD_EXAMPLE (stack_example)
target_link_libraries (stack_example SuperStupidStack CORSIKAunits
  CORSIKAlogging)

# address sanitizer is making this example too slow, so we only do "undefined"
CORSIKA_ADD_EXAMPLE (cascade_example)
target_link_libraries (boundary_example
  SuperStupidStack
  CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  CORSIKAcascade
  ProcessTrackWriter
  ProcessParticleCut
  ProcessTrackingLine
  CORSIKAprocesses
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )
install (TARGETS boundary_example DESTINATION share/examples)

if (Pythia8_FOUND)
  CORSIKA_ADD_EXAMPLE (cascade_proton_example)
  target_link_libraries (cascade_proton_example
    SuperStupidStack
    CORSIKAunits
    CORSIKAlogging
    CORSIKArandom
    ProcessSibyll
    ProcessPythia
    CORSIKAcascade
    ProcessEnergyLoss
    ProcessTrackWriter
    ProcessStackInspector
    ProcessTrackingLine
    ProcessParticleCut
    ProcessHadronicElasticModel
    ProcessStackInspector
    CORSIKAprocesses
    CORSIKAcascade
    CORSIKAparticles
    CORSIKAgeometry
    CORSIKAenvironment
    CORSIKAprocesssequence
    )
  install (TARGETS cascade_proton_example DESTINATION share/examples)

  CORSIKA_ADD_EXAMPLE (vertical_EAS)
  target_link_libraries (vertical_EAS
    SuperStupidStack
    CORSIKAunits
    CORSIKAlogging
    CORSIKArandom
    ProcessSibyll
    ProcessPythia
    ProcessUrQMD
    ProcessSwitch
    CORSIKAcascade
    ProcessEnergyLoss
    ProcessObservationPlane
    ProcessInteractionCounter
    ProcessTrackWriter
    ProcessTrackingLine
    ProcessParticleCut
    ProcessStackInspector
    CORSIKAprocesses
    CORSIKAcascade
    CORSIKAparticles
    CORSIKAgeometry
    CORSIKAenvironment
    CORSIKAprocesssequence
    )
  install (TARGETS vertical_EAS DESTINATION share/examples)
endif()

CORSIKA_ADD_EXAMPLE (stopping_power stopping_power)
target_link_libraries (stopping_power
  SuperStupidStack
  CORSIKAunits
  ProcessEnergyLoss
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  )
install (TARGETS stopping_power DESTINATION share/examples)

CORSIKA_ADD_EXAMPLE (staticsequence_example)
target_link_libraries (staticsequence_example
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAlogging)
install (TARGETS staticsequence_example DESTINATION share/examples)
