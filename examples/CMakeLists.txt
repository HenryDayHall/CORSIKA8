add_executable (helix_example helix_example.cpp)
target_link_libraries (helix_example CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (helix_example)

add_executable (geometry_example geometry_example.cpp)
target_link_libraries (geometry_example CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (geometry_example)

# # logging back in March was not used
# #add_executable (logger_example logger_example.cpp)
# #target_link_libraries (logger_example CORSIKA8)
# #CORSIKA_REGISTER_EXAMPLE (logger_example)

# add_executable (stack_example stack_example.cpp)
# target_link_libraries (stack_example CORSIKA8)
# CORSIKA_REGISTER_EXAMPLE (stack_example)

# add_executable (cascade_example cascade_example.cpp)
# target_link_libraries (cascade_example CORSIKA8)
# CORSIKA_REGISTER_EXAMPLE (cascade_example)

add_executable (boundary_example boundary_example.cpp)
target_link_libraries (boundary_example CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (boundary_example)

# add_executable (cascade_proton_example cascade_proton_example.cpp)
# target_link_libraries (cascade_proton_example CORSIKA8)
# CORSIKA_REGISTER_EXAMPLE (cascade_proton_example)

# add_executable (vertical_EAS vertical_EAS.cpp)
# target_link_libraries (vertical_EAS CORSIKA8)
# CORSIKA_REGISTER_EXAMPLE (vertical_EAS)

add_executable (stopping_power stopping_power.cpp)
target_link_libraries (stopping_power CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (stopping_power)

add_executable (staticsequence_example staticsequence_example.cpp)
target_link_libraries (staticsequence_example CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (staticsequence_example)

add_executable (particle_list_example particle_list_example.cpp)
target_link_libraries (particle_list_example CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (particle_list_example)
