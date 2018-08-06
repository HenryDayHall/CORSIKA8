

add_executable (geometry_example geometry_example.cc)

target_link_libraries (geometry_example CORSIKAgeometry CORSIKAunits)

install (TARGETS geometry_example DESTINATION bin)
