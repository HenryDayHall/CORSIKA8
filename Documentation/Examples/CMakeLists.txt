CORSIKA_ADD_TEST (helix_example)
target_link_libraries (helix_example CORSIKAgeometry CORSIKAunits)
install (TARGETS helix_example DESTINATION share/examples)

CORSIKA_ADD_TEST (geometry_example)
target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)
install (TARGETS geometry_example DESTINATION share/examples)

CORSIKA_ADD_TEST (logger_example)
target_link_libraries (logger_example CORSIKAunits CORSIKAlogging)
install (TARGETS logger_example DESTINATION share/examples)

CORSIKA_ADD_TEST (stack_example)
target_link_libraries (stack_example SuperStupidStack CORSIKAunits
  CORSIKAlogging)

# address sanitizer is making this example too slow, so we only do "undefined"
CORSIKA_ADD_TEST (cascade_example SANITIZE "undefined")
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

CORSIKA_ADD_TEST (boundary_example)
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
  CORSIKA_ADD_TEST (cascade_proton_example)
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
endif()

CORSIKA_ADD_TEST(vertical_EAS)
target_link_libraries (vertical_EAS
  SuperStupidStack
  CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  CORSIKAcascade
  ProcessEnergyLoss
  ProcessTrackWriter
  ProcessTrackingLine
  ProcessHadronicElasticModel
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

CORSIKA_ADD_TEST(stopping_power)
target_link_libraries (stopping_power
  SuperStupidStack
  CORSIKAunits
  ProcessEnergyLoss
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  )

install (TARGETS vertical_EAS DESTINATION share/examples)
CORSIKA_ADD_TEST (staticsequence_example)
target_link_libraries (staticsequence_example
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAlogging)
install (TARGETS staticsequence_example DESTINATION share/examples)
