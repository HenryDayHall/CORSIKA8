find_package (Doxygen OPTIONAL_COMPONENTS dot mscgen dia)

if (DOXYGEN_FOUND)
  if (NOT DOXYGEN_DOT_EXECUTABLE)
    message (FATAL_ERROR "Found doxygen but not 'dot' command, please install graphviz or set DOXYGEN_DOT_EXECUTABLE")
  endif()

  set (DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set (DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  set (DOXYGEN_GENERATE_HTML YES)
  set (DOXYGEN_GENERATE_MAN YES)
  configure_file (${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message ("Start doxygen with \"make doxygen\"")
  
  # note the option ALL which allows to build the docs together with the application
  add_custom_target (doxygen # ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

  add_custom_command(TARGET doxygen POST_BUILD
    COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/latex; pdflatex refman.tex
    )
  
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc OPTIONAL)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf DESTINATION share/doc OPTIONAL)
         
else (DOXYGEN_FOUND)
  
  message ("Doxygen need to be installed to generate the doxygen documentation")
  
endif (DOXYGEN_FOUND)
