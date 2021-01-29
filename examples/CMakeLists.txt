cmake_minimum_required (VERSION 3.9)

project (CORSIKA8_examples)
set (CMAKE_VERBOSE_MAKEFILE OFF) # this can be changed with `make VERBOSE=1`

# this is enough to use CORSIKA8
find_package (CORSIKA8 CONFIG REQUIRED)

# this is only for CORSIKA_REGISTER_EXAMPLE
include ("${CMAKE_CURRENT_SOURCE_DIR}/CMakeHelper.cmake") 

add_executable (helix_example helix_example.cpp)
target_link_libraries (helix_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (helix_example)

add_executable (geometry_example geometry_example.cpp)
target_link_libraries (geometry_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (geometry_example)

add_executable (stack_example stack_example.cpp)
target_link_libraries (stack_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (stack_example)

add_executable (cascade_example cascade_example.cpp)
target_link_libraries (cascade_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (cascade_example)

add_executable (boundary_example boundary_example.cpp)
target_link_libraries (boundary_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (boundary_example)

add_executable (cascade_proton_example cascade_proton_example.cpp)
target_link_libraries (cascade_proton_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (cascade_proton_example)

add_executable (vertical_EAS vertical_EAS.cpp)
target_link_libraries (vertical_EAS CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (vertical_EAS RUN_OPTIONS 4 2 10000.)

add_executable (stopping_power stopping_power.cpp)
target_link_libraries (stopping_power CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (stopping_power)

add_executable (staticsequence_example staticsequence_example.cpp)
target_link_libraries (staticsequence_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (staticsequence_example)

add_executable (particle_list_example particle_list_example.cpp)
target_link_libraries (particle_list_example CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (particle_list_example)

add_executable (em_shower em_shower.cpp)
target_link_libraries (em_shower CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (em_shower RUN_OPTIONS "100.")

add_executable (hybrid_MC hybrid_MC.cpp)
target_link_libraries (hybrid_MC  CORSIKA8::CORSIKA8)
CORSIKA_REGISTER_EXAMPLE (hybrid_MC RUN_OPTIONS 4 2 10000.)
