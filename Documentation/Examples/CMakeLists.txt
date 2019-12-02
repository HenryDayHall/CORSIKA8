
CORSIKA_ADD_EXAMPLE (helix_example)
target_link_libraries (helix_example CORSIKAgeometry CORSIKAunits)

CORSIKA_ADD_EXAMPLE (particle_list_example)
target_link_libraries (particle_list_example CORSIKAparticles CORSIKAunits CORSIKAprocesses ProcessSibyll ProcessQGSJetII)

CORSIKA_ADD_EXAMPLE (geometry_example)
target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)

CORSIKA_ADD_EXAMPLE (stack_example)
target_link_libraries (stack_example SuperStupidStack CORSIKAunits)

# address sanitizer is making this example too slow, so we only do "undefined"
CORSIKA_ADD_EXAMPLE (boundary_example)
target_link_libraries (boundary_example
    SuperStupidStack
    CORSIKAunits
    CORSIKAlogging
    CORSIKArandom
    ProcessSibyll
    ProcessPythia8
    ProcessProposal
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

CORSIKA_ADD_EXAMPLE (cascade_example)
target_link_libraries (cascade_example
    SuperStupidStack
    CORSIKAunits
    CORSIKAlogging
    CORSIKArandom
    ProcessSibyll
    ProcessPythia8
    ProcessUrQMD
    ProcessSwitch
    CORSIKAcascade
    ProcessCONEXSourceCut
    ProcessEnergyLoss
    ProcessTrackWriter
    ProcessStackInspector
    ProcessTrackingLine
    ProcessProposal
    ProcessParticleCut
    ProcessOnShellCheck
    ProcessHadronicElasticModel
    ProcessStackInspector
    CORSIKAprocesses
    CORSIKAcascade
    CORSIKAparticles
    CORSIKAgeometry
    CORSIKAenvironment
    CORSIKAprocesssequence
    )

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

    CORSIKA_ADD_EXAMPLE (vertical_EAS RUN_OPTIONS 4 2 10000.)
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
        ProcessLongitudinalProfile
        CORSIKAprocesses
        CORSIKAcascade
        CORSIKAparticles
        CORSIKAgeometry
        CORSIKAenvironment
        CORSIKAprocesssequence
        )
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

CORSIKA_ADD_EXAMPLE (staticsequence_example)
target_link_libraries (staticsequence_example
    CORSIKAprocesssequence
    CORSIKAunits
    CORSIKAgeometry
    CORSIKAlogging)

CORSIKA_ADD_EXAMPLE (proposal_example RUN_OPTIONS 100.)
target_link_libraries (proposal_example
    SuperStupidStack
    CORSIKAunits
    CORSIKAlogging
    CORSIKArandom
    CORSIKAcascade
    ProcessObservationPlane
    ProcessInteractionCounter
    ProcessTrackWriter
    ProcessProposal
    ProcessTrackingLine
    ProcessParticleCut
    ProcessStackInspector
    ProcessLongitudinalProfile
    CORSIKAprocesses
    CORSIKAcascade
    CORSIKAparticles
    CORSIKAgeometry
    CORSIKAenvironment
    CORSIKAprocesssequence
    )
