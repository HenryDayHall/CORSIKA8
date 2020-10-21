#
# (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
#
# See file AUTHORS for a list of contributors.
#
# This software is distributed under the terms of the GNU General Public
# Licence version 3 (GPL Version 3). See file LICENSE for a full version of
# the license.
#

#################################################
#
# central macro to register unit tests in cmake
#
# 1) Simple use:
# Pass the name of the test.cc file as the first
# argument, without the ".cc" extention.
#
# Example: CORSIKA_ADD_TEST (testSomething)
#
# This generates target testSomething from file testSomething.cc.
#
# 2) Customize sources:
# If 1) doesn't work, use the SOURCES keyword to explicitly
# specify the sources.
#
# Example: CORSIKA_ADD_TEST (testSomething
#              SOURCES source1.cc source2.cc someheader.h)
#
# 3) Customize sanitizers:
# You can override which sanitizers are compiled into the
# test, but only do this if the defaults do not work.
#
# Example: CORSIKA_ADD_TEST (testSomething SANITIZE undefined)
#
# Only uses the sanitizer for undefined behavior.
#
# In all cases, you can further customize the target with
# target_link_libraries(testSomething ...) and so on.
#
# TEMPORARY: All sanitizers are currently globally disabled by default, to enable them,
# set CORSIKA_SANITIZERS_ENABLED to TRUE.
function (CORSIKA_ADD_TEST)
  cmake_parse_arguments (PARSE_ARGV 1 C8_ADD_TEST "" "SANITIZE" "SOURCES")
  set (name ${ARGV0})

  if (NOT C8_ADD_TEST_SOURCES)
    set (sources ${name}.cc)
  else ()
    set (sources ${C8_ADD_TEST_SOURCES})
  endif ()

  if (NOT C8_ADD_TEST_SANITIZE)
    set(sanitize "address,undefined")
  else ()
    set(sanitize ${C8_ADD_TEST_SANITIZE})
  endif ()

  add_executable (${name} ${sources})
  target_link_libraries (${name} CORSIKA8 Catch2)
  target_compile_options (${name} PRIVATE -g) # do not skip asserts
  target_include_directories (${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/test_outputs/)
  if (CORSIKA_SANITIZERS_ENABLED)
    # -O1 is suggested in clang docs to get reasonable performance
    target_compile_options (${name} PRIVATE -O1 -fno-omit-frame-pointer -fsanitize=${sanitize} -fno-sanitize-recover=all)
    set_target_properties (${name} PROPERTIES LINK_FLAGS "-fsanitize=${sanitize}")
  endif ()
  add_test (NAME ${name} COMMAND ${name} -o ${PROJECT_BINARY_DIR}/test_outputs/junit-${name}.xml -s -r junit)
endfunction (CORSIKA_ADD_TEST)


#################################################
#
# central macro to register an examples in cmake
#
# Examples can be globally executed by 'make run_examples'
#
# 1) Simple use:
# Pass the name of the test.cc file as the first
# argument, without the ".cc" extention.
#
# Example: CORSIKA_REGISTER_EXAMPLE (doSomething)
#
# The TARGET doSomething must already exists,
#    i.e. typically via add_executable (doSomething src.cc).
#
# Example: CORSIKA_ADD_EXAMPLE (testSomething
#              RUN_OPTION "extra command line options"
#              )
#
# In all cases, you can further customize the target with
# target_link_libraries(testSomething ...) and so on.
#
function (CORSIKA_REGISTER_EXAMPLE)
  cmake_parse_arguments (PARSE_ARGV 1 C8_EXAMPLE "" "" "RUN_OPTIONS")
  set (name ${ARGV0})

  if (NOT C8_EXAMPLE_RUN_OPTIONS)
    set (run_options "")
  else ()
    set (run_options ${C8_EXAMPLE_RUN_OPTIONS})
  endif ()

  target_compile_options (${name} PRIVATE -g) # do not skip asserts
  file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/example_outputs/)
  if (TARGET run_examples)
  else ()
    add_custom_target (run_examples)
  endif ()
  add_dependencies (run_examples ${name})
  # just run the command as-is
  set (CMD ${CMAKE_CURRENT_BINARY_DIR}/${name} ${run_options})
  add_custom_command (TARGET run_examples
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "**************************************"
    COMMAND ${CMAKE_COMMAND} -E echo "*****   running example: ${name} " ${run_options} VERBATIM
    COMMAND ${CMD} VERBATIM
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/example_outputs)
  install (TARGETS ${name} DESTINATION share/examples)
endfunction (CORSIKA_REGISTER_EXAMPLE)
