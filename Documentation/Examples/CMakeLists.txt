
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

CORSIKA_ADD_EXAMPLE (stack_example stack_example.cc)
target_link_libraries (stack_example SuperStupidStack CORSIKAunits
  CORSIKAlogging)

# address sanitizer is making this example too slow, so we only do "undefined"
CORSIKA_ADD_EXAMPLE (cascade_example cascade_example.cc)
target_link_libraries (cascade_example
  SuperStupidStack
  CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  CORSIKAcascade
  ProcessEnergyLoss
  ProcessStackInspector
  ProcessParticleCut
  ProcessTrackWriter
  ProcessTrackingLine
  CORSIKAprocesses
  CORSIKAcascade
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )
install (TARGETS cascade_example DESTINATION share/examples)

CORSIKA_ADD_EXAMPLE (boundary_example boundary_example.cc)
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
  CORSIKA_ADD_EXAMPLE (cascade_proton_example cascade_proton_example.cc)
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

<<<<<<< Updated upstream
  add_executable(vertical_EAS vertical_EAS.cc)
=======
  CORSIKA_ADD_EXAMPLE (vertical_EAS vertical_EAS.cc)
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    ProcessTrackingLine
    ProcessParticleCut
    ProcessStackInspector
    ProcessInteractionCounter
=======
    ProcessTrackWriter
    ProcessTrackingLine
    ProcessParticleCut
    ProcessStackInspector
>>>>>>> Stashed changes
    CORSIKAprocesses
    CORSIKAcascade
    CORSIKAparticles
    CORSIKAgeometry
    CORSIKAenvironment
    CORSIKAprocesssequence
    )
  install (TARGETS vertical_EAS DESTINATION share/examples)
<<<<<<< Updated upstream
  
=======
>>>>>>> Stashed changes
endif()

CORSIKA_ADD_EXAMPLE (stopping_power stopping_power.cc)
target_link_libraries (stopping_power
  SuperStupidStack
  CORSIKAunits
  ProcessEnergyLoss
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  )
install (TARGETS stopping_power DESTINATION share/examples)

<<<<<<< Updated upstream
CORSIKA_ADD_TEST (staticsequence_example)
=======
CORSIKA_ADD_EXAMPLE (staticsequence_example staticsequence_example.cc)
>>>>>>> Stashed changes
target_link_libraries (staticsequence_example
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAlogging)
install (TARGETS staticsequence_example DESTINATION share/examples)
