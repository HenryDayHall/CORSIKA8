
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
function (CORSIKA_COPY_HEADERS_TO_NAMESPACE for_library in_namespace)
  CORSIKA_PREPEND_PATH (HEADERS_BUILD "${PROJECT_BINARY_DIR}/include/${in_namespace}" ${ARGN})
  foreach (HEADER ${ARGN})
    add_custom_command (
      OUTPUT ${PROJECT_BINARY_DIR}/include/${in_namespace}/${HEADER}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/include/${in_namespace}
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/${HEADER} ${PROJECT_BINARY_DIR}/include/${in_namespace}/${HEADER}
      COMMENT "Generate link: ${PROJECT_BINARY_DIR}/include/${in_namespace}/${HEADER}"
      #      VERBATIM
      )
  endforeach (HEADER)
  
  # main target for this library, depends on header files in build area
  add_custom_target (
    ${for_library}_BUILD
    DEPENDS ${HEADERS_BUILD}
    )
  
  add_dependencies (${for_library} ${for_library}_BUILD)

endfunction (CORSIKA_COPY_HEADERS_TO_NAMESPACE)

