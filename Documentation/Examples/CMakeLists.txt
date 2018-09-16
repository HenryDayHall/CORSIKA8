add_executable (helix_example helix_example.cc)
target_link_libraries(helix_example CORSIKAgeometry CORSIKAunits)
install (TARGETS helix_example DESTINATION share/examples)

add_executable (geometry_example geometry_example.cc)
target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)
install (TARGETS geometry_example DESTINATION share/examples)

add_executable (logger_example logger_example.cc)
target_link_libraries (logger_example CORSIKAunits CORSIKAlogging)
install (TARGETS logger_example DESTINATION share/examples)

add_executable (stack_example stack_example.cc)
target_link_libraries (stack_example SuperStupidStack CORSIKAunits CORSIKAlogging)
install (TARGETS stack_example DESTINATION share/examples)

add_executable (staticsequence_example staticsequence_example.cc)
target_link_libraries (staticsequence_example
  CORSIKAprocesssequence
  CORSIKAunits
  CORSIKAlogging)
install (TARGETS staticsequence_example DESTINATION share/examples)

