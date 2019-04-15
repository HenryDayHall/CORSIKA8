add_executable (helix_example helix_example.cc)
target_compile_options(helix_example PRIVATE -g) # do not skip asserts
target_link_libraries(helix_example CORSIKAgeometry CORSIKAunits)
install (TARGETS helix_example DESTINATION share/examples)
CORSIKA_ADD_TEST (helix_example)

add_executable (geometry_example geometry_example.cc)
target_compile_options(geometry_example PRIVATE -g) # do not skip asserts
target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)
install (TARGETS geometry_example DESTINATION share/examples)
CORSIKA_ADD_TEST (geometry_example)

add_executable (logger_example logger_example.cc)
target_compile_options(logger_example PRIVATE -g) # do not skip asserts
target_link_libraries (logger_example CORSIKAunits CORSIKAlogging)
install (TARGETS logger_example DESTINATION share/examples)
CORSIKA_ADD_TEST (logger_example)

add_executable (stack_example stack_example.cc)
target_compile_options(stack_example PRIVATE -g) # do not skip asserts
target_link_libraries (stack_example SuperStupidStack CORSIKAunits
  CORSIKAlogging)
CORSIKA_ADD_TEST (stack_example)

add_executable (cascade_example cascade_example.cc)
target_compile_options(cascade_example PRIVATE -g) # do not skip asserts
target_link_libraries (cascade_example SuperStupidStack CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  ProcessPythia
  CORSIKAcascade
  ProcessEnergyLoss
  ProcessStackInspector
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
CORSIKA_ADD_TEST (cascade_example)

add_executable (boundary_example boundary_example.cc)
target_compile_options(boundary_example PRIVATE -g) # do not skip asserts
target_link_libraries (boundary_example SuperStupidStack CORSIKAunits CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  CORSIKAcascade
  ProcessTrackWriter
  ProcessTrackingLine
  CORSIKAprocesses
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )

install (TARGETS boundary_example DESTINATION share/examples)
CORSIKA_ADD_TEST (boundary_example)

add_executable (cascade_proton_example cascade_proton_example.cc)
target_compile_options(cascade_proton_example PRIVATE -g) # do not skip asserts
target_link_libraries (cascade_proton_example SuperStupidStack CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  ProcessPythia
  CORSIKAcascade
  ProcessEnergyLoss
  ProcessTrackWriter
  ProcessTrackingLine
  ProcessHadronicElasticModel
  CORSIKAprocesses
  CORSIKAcascade
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )

install (TARGETS cascade_proton_example DESTINATION share/examples)
CORSIKA_ADD_TEST (cascade_proton_example)

add_executable (vertical_EAS vertical_EAS.cc)
target_compile_options(vertical_EAS PRIVATE -g) # do not skip asserts
target_link_libraries (vertical_EAS SuperStupidStack CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  ProcessPythia
  CORSIKAcascade
  ProcessEnergyLoss
  ProcessTrackWriter
  ProcessTrackingLine
  ProcessHadronicElasticModel
  CORSIKAprocesses
  CORSIKAcascade
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )
install (TARGETS vertical_EAS DESTINATION share/examples)
# CORSIKA_ADD_TEST (vertical_EAS) not yet...

add_executable (staticsequence_example staticsequence_example.cc)
target_compile_options(staticsequence_example PRIVATE -g) # do not skip asserts
target_link_libraries (staticsequence_example
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAlogging)
install (TARGETS staticsequence_example DESTINATION share/examples)
CORSIKA_ADD_TEST (staticsequence_example)
