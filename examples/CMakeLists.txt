add_executable (helix_example helix_example.cpp)
target_link_libraries (helix_example CORSIKA8)

add_executable (geometry_example geometry_example.cpp)
target_link_libraries (geometry_example CORSIKA8)

# loggin back in March was not used
#add_executable (logger_example logger_example.cpp)
#target_link_libraries (logger_example CORSIKA8)

add_executable (stack_example stack_example.cpp)
target_link_libraries (stack_example CORSIKA8)

add_executable (cascade_example cascade_example.cpp)
target_link_libraries (cascade_example CORSIKA8)

add_executable (boundary_example boundary_example.cpp)
target_link_libraries (boundary_example CORSIKA8)

add_executable (cascade_proton_example cascade_proton_example.cpp)
target_link_libraries (cascade_proton_example CORSIKA8)

add_executable (vertical_EAS vertical_EAS.cpp)
target_link_libraries (vertical_EAS CORSIKA8)

add_executable (stopping_power stopping_power.cpp)
target_link_libraries (stopping_power CORSIKA8)

add_executable (staticsequence_example staticsequence_example.cpp)
target_link_libraries (staticsequence_example CORSIKA8)
