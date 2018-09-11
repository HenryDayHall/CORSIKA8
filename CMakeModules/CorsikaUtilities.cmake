
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

