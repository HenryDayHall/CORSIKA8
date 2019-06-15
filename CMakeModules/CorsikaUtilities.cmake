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
# takes a list of input files and prepends a path
#

function (CORSIKA_PREPEND_PATH return prefix)
  set (listVar "")
  foreach (f ${ARGN})
    list (APPEND listVar "${prefix}/${f}")
  endforeach (f)
  set (${return} "${listVar}" PARENT_SCOPE)
endfunction (CORSIKA_PREPEND_PATH)


#################################################
#
# use: CORSIKA_COPY_HEADERS_TO_NAMESPACE theLib theNamesapce header1.h header2.h ...
#
# creates a dependence of theLib on the presence of all listed header files in the
# build-directory include/theNamespace directory
#
# if needed, create symbolic links from the source files to this build-directory location
#
# any path information from input filenames is stripped, IF path was specified it is used for the link destination, if NOT the link is relative to the CMAKE_CURRENT_SOURCE_DIR
# 
function (CORSIKA_COPY_HEADERS_TO_NAMESPACE for_library in_namespace)
  set (HEADERS_BUILD "")
  foreach (HEADER ${ARGN})
    # find eventual path, and handle it specificly
    get_filename_component (barename ${HEADER} NAME)
    get_filename_component (baredir ${HEADER} DIRECTORY)
    # remove path, prepend build-directory destination
    list (APPEND HEADERS_BUILD "${PROJECT_BINARY_DIR}/include/${in_namespace}/${barename}")
    # define source location, use path if specified, otherwise CMAKE_CURRENT_SOURCE_DIR
    set (FROM_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    if (NOT "${baredir}" STREQUAL "")
      set (FROM_DIR ${baredir})
    endif ()
    # define command to perform the symbolic linking
    add_custom_command (
      OUTPUT ${PROJECT_BINARY_DIR}/include/${in_namespace}/${barename}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/include/${in_namespace}
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${FROM_DIR}/${barename} ${PROJECT_BINARY_DIR}/include/${in_namespace}/${barename}
      COMMENT "Generate link: ${PROJECT_BINARY_DIR}/include/${in_namespace}/${barename}"
      )
  endforeach (HEADER)
  
  # main target for this build step, depends on header files in build area
  add_custom_target (
    ${for_library}_BUILD
    DEPENDS ${HEADERS_BUILD}
    )
  
  # connect the _BUILD target to the main (external) target
  add_dependencies (${for_library} ${for_library}_BUILD)

endfunction (CORSIKA_COPY_HEADERS_TO_NAMESPACE)




#################################################
#
# use: CORSIKA_ADD_FILES_ABSOLUTE varname
#
# add list of filenames with absolute paths (pointing to CMAKE_SOURCE_DIR) to ${varname} in PARAENT_SCOPE
# 

macro (CORSIKA_ADD_FILES_ABSOLUTE varname)
  file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  foreach (_src ${ARGN})
    if (_relPath)
      list (APPEND "${varname}" "${CMAKE_SOURCE_DIR}/${_relPath}/${_src}")
    else()
      list (APPEND "${varname}" "${CMAKE_SOURCE_DIR}/${_src}")
    endif()
  endforeach()
  if (_relPath)
    # propagate SRCS to parent directory
    set ("${varname}" "${${varname}}" PARENT_SCOPE)
  endif()
endmacro(CORSIKA_ADD_FILES_ABSOLUTE)



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
  cmake_parse_arguments (PARSE_ARGV 1 _ "" "SANITIZE" "SOURCES")

  set (name ${ARGV0})

  if (NOT __SOURCES)
    set (sources ${name}.cc)
  else ()
    set (sources ${__SOURCES})
  endif ()

  if (NOT __SANITIZE)
    set(sanitize "address,undefined")
  else ()
    set(sanitize ${__SANITIZE})
  endif ()

  add_executable (${name} ${sources})
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
