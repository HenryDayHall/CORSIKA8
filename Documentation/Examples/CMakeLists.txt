
add_executable (geometry_example geometry_example.cc)
target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)
install (TARGETS geometry_example DESTINATION share/examples)

add_executable (logger_example logger_example.cc)
target_link_libraries (logger_example CORSIKAunits CORSIKAlogging)
install (TARGETS logger_example DESTINATION share/examples)

add_executable (stack_example stack_example.cc)
target_link_libraries (stack_example CORSIKAstack CORSIKAunits CORSIKAlogging)
install (TARGETS stack_example DESTINATION share/examples)

