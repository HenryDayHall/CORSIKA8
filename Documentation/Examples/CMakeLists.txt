
CORSIKA_ADD_EXAMPLE (helix_example)
target_link_libraries (helix_example CORSIKAgeometry CORSIKAunits)

CORSIKA_ADD_EXAMPLE (particle_list_example)
target_link_libraries (particle_list_example CORSIKAparticles CORSIKAunits CORSIKAprocesses ProcessSibyll ProcessQGSJetII)

CORSIKA_ADD_EXAMPLE (geometry_example)
target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)

CORSIKA_ADD_EXAMPLE (stack_example)
target_link_libraries (stack_example CORSIKAsetup CORSIKAunits)

# address sanitizer is making this example too slow, so we only do "undefined"
CORSIKA_ADD_EXAMPLE (boundary_example)
target_link_libraries (boundary_example
  CORSIKAsetup
  CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  CORSIKAcascade
  ProcessTrackWriter
  ProcessParticleCut
  ProcessTrackingLine
  ProcessPythia8
  CORSIKAprocesses
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAoutput
  CORSIKAprocesssequence
  C8::ext::boost # boost::histogram
  )

CORSIKA_ADD_EXAMPLE (cascade_example)
target_link_libraries (cascade_example
  CORSIKAsetup
  CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  ProcessProposal
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
  CORSIKAoutput
  CORSIKAprocesssequence
  )

if (Pythia8_FOUND)
  CORSIKA_ADD_EXAMPLE (cascade_proton_example)
  target_link_libraries (cascade_proton_example
    CORSIKAsetup
    CORSIKAunits
    CORSIKAlogging
    CORSIKArandom
    ProcessSibyll
    ProcessPythia8
    ProcessUrQMD
    CORSIKAcascade
    ProcessCONEXSourceCut
    ProcessEnergyLoss
    ProcessTrackWriter
    ProcessStackInspector
    ProcessTrackingLine
    ProcessParticleCut
    ProcessOnShellCheck
    ProcessHadronicElasticModel
    ProcessStackInspector
    CORSIKAprocesses
    CORSIKAcascade
    CORSIKAparticles
    CORSIKAgeometry
    CORSIKAoutput
    CORSIKAenvironment
    CORSIKAprocesssequence
    )

  CORSIKA_ADD_EXAMPLE (vertical_EAS RUN_OPTIONS 4 2 10000.)
  target_link_libraries (vertical_EAS
    CORSIKAsetup
    CORSIKAunits
    CORSIKAlogging
    CORSIKArandom
    CORSIKAhistory
    ProcessSibyll
    ProcessPythia8
    ProcessUrQMD
    CORSIKAcascade
    ProcessProposal
    ProcessPythia8
    ProcessObservationPlane
    ProcessInteractionCounter
    ProcessTrackWriter
    ProcessEnergyLoss
    ProcessTrackingLine
    ProcessParticleCut
    ProcessOnShellCheck
    ProcessStackInspector
    ProcessLongitudinalProfile
    ProcessCONEXSourceCut
    CORSIKAprocesses
    CORSIKAcascade
    CORSIKAparticles
    CORSIKAgeometry
    CORSIKAoutput
    CORSIKAenvironment
    CORSIKAprocesssequence
    CORSIKAhistory # for HistoryObservationPlane
    )
endif()

CORSIKA_ADD_EXAMPLE (stopping_power stopping_power)
target_link_libraries (stopping_power
  CORSIKAsetup
  CORSIKAunits
  CORSIKAlogging
  ProcessEnergyLoss
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAoutput
  CORSIKAenvironment
  )

CORSIKA_ADD_EXAMPLE (staticsequence_example)
target_link_libraries (staticsequence_example
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAoutput
  CORSIKAgeometry
  CORSIKAlogging)


CORSIKA_ADD_EXAMPLE (em_shower RUN_OPTIONS "100.")
target_link_libraries (em_shower
  SuperStupidStack
  CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  ProcessPythia8
  ProcessUrQMD
  CORSIKAcascade
  ProcessCONEXSourceCut
  ProcessEnergyLoss
  ProcessObservationPlane
  ProcessInteractionCounter
  ProcessTrackWriter
  ProcessProposal
  ProcessTrackingLine
  ProcessParticleCut
  ProcessOnShellCheck
  ProcessStackInspector
  ProcessLongitudinalProfile
  CORSIKAprocesses
  CORSIKAcascade
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )
