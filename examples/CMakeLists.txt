cmake_minimum_required (VERSION 3.9)

project (CORSIKA8_examples)
set (CMAKE_VERBOSE_MAKEFILE OFF) # this can be changed with `make VERBOSE=1`

# this is enough to use CORSIKA8
find_package (corsika CONFIG REQUIRED)

# this is only for CORSIKA_REGISTER_EXAMPLE
include ("${CMAKE_CURRENT_SOURCE_DIR}/corsikaExamples.cmake")

find_package(Catch2)
find_package(yaml-cpp)
find_package(CLI11)
find_package(Arrow)
find_package(PROPOSAL)
find_package(Boost)
find_package(ZLIB)
find_package(BZip2)
find_package(Eigen3)
find_package(spdlog)

set(CONANDEPS  Catch2::Catch2WithMain  yaml-cpp::yaml-cpp  CLI11::CLI11  arrow::arrow  PROPOSAL::PROPOSAL  boost::boost  ZLIB::ZLIB  BZip2::BZip2  Eigen3::Eigen  spdlog::spdlog )

###################
## cascade_examples
###################

add_executable (em_shower cascade_examples/em_shower.cpp)
target_link_libraries (em_shower CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (em_shower RUN_OPTIONS 100 8472)

if (WITH_FLUKA)
  add_executable (mars cascade_examples/mars.cpp)
  target_link_libraries (mars CORSIKA8::CORSIKA8 ${CONANDEPS})
  message("FLUKA found, will make 'mars' example")
else()
  message("FLUKA not found, will not make 'mars' example")
endif()

add_executable (mc_conex cascade_examples/mc_conex.cpp)
target_link_libraries (mc_conex  CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (mc_conex RUN_OPTIONS 4 2 10000.)

add_executable (radio_em_shower cascade_examples/radio_em_shower.cpp)
target_link_libraries (radio_em_shower CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (radio_em_shower RUN_OPTIONS 10 1121673489)

if (WITH_FLUKA)
  add_executable (water cascade_examples/water.cpp)
  target_link_libraries (water CORSIKA8::CORSIKA8 ${CONANDEPS})
  message("FLUKA found, will make 'water' example")
else()
  message("FLUKA not found, will not make 'water' example")
endif()


#####################
## framework_examples
#####################

add_executable (boundary_crossing framework_examples/boundary_crossing.cpp)
target_link_libraries (boundary_crossing CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (boundary_crossing)

add_executable (environment framework_examples/environment.cpp)
target_link_libraries (environment CORSIKA8::CORSIKA8 ${CONANDEPS})

add_executable (geometry framework_examples/geometry.cpp)
target_link_libraries (geometry CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (geometry)

add_executable (helix_trajectory framework_examples/helix_trajectory.cpp)
target_link_libraries (helix_trajectory CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (helix_trajectory)

add_executable (known_particles framework_examples/known_particles.cpp)
target_link_libraries (known_particles CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (known_particles)

add_executable (stack framework_examples/stack.cpp)
target_link_libraries (stack CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (stack)

add_executable (static_sequence framework_examples/static_sequence.cpp)
target_link_libraries (static_sequence CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (static_sequence)


###################
## physics_examples
###################

add_executable (stopping_power physics_examples/stopping_power.cpp)
target_link_libraries (stopping_power CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (stopping_power)

add_executable (synchrotron_clover_leaf physics_examples/synchrotron_clover_leaf.cpp)
target_link_libraries (synchrotron_clover_leaf CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (synchrotron_clover_leaf)

add_executable (synchrotron_test_C8tracking physics_examples/synchrotron_test_C8tracking.cpp)
target_link_libraries (synchrotron_test_C8tracking CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (synchrotron_test_C8tracking)

add_executable (synchrotron_test_manual_tracking physics_examples/synchrotron_test_manual_tracking.cpp)
target_link_libraries (synchrotron_test_manual_tracking CORSIKA8::CORSIKA8 ${CONANDEPS})
CORSIKA_REGISTER_EXAMPLE (synchrotron_test_manual_tracking)
