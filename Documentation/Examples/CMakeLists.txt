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
  ProcessStackInspector
  ProcessTrackWriter
  ProcessTrackingLine
  ProcessHadronicElasticModel
  CORSIKAprocesses
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )
install (TARGETS cascade_example DESTINATION share/examples)
CORSIKA_ADD_TEST (cascade_example)

add_executable (cascade_proton_example cascade_proton_example.cc)
target_compile_options(cascade_proton_example PRIVATE -g) # do not skip asserts
target_link_libraries (cascade_proton_example SuperStupidStack CORSIKAunits
  CORSIKAlogging
  CORSIKArandom
  ProcessSibyll
  ProcessPythia
  CORSIKAcascade
  ProcessStackInspector
  ProcessTrackWriter
  ProcessTrackingLine
  ProcessHadronicElasticModel
  CORSIKAprocesses
  CORSIKAparticles
  CORSIKAgeometry
  CORSIKAenvironment
  CORSIKAprocesssequence
  )
install (TARGETS cascade_proton_example DESTINATION share/examples)
CORSIKA_ADD_TEST (cascade_proton_example)

add_executable (staticsequence_example staticsequence_example.cc)
target_compile_options(staticsequence_example PRIVATE -g) # do not skip asserts
target_link_libraries (staticsequence_example
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAgeometry
  CORSIKAlogging)
install (TARGETS staticsequence_example DESTINATION share/examples)
CORSIKA_ADD_TEST (staticsequence_example)
