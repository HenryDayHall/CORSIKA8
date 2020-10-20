
set (test_modules_sources
  TestMain.cpp
  testSibyll.cpp
  testNullModel.cpp
  testObservationPlane.cpp
  #testParticleCut.cpp
  #testPythia.cpp
  #testQGSJetII.cpp
  #testSibyll.cpp
  #testStackInspector.cpp
  #testSwitchProcess.cpp
  #testTrackingLine.cpp
  #testUrQMD.cpp
  )

add_executable (testModules ${test_modules_sources})

target_link_libraries (testModules CORSIKA8 Sibyll_static Catch2)

target_compile_options (testModules PRIVATE -g) # do not skip asserts

target_include_directories (testModules PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/test_outputs/)

if (CORSIKA_SANITIZERS_ENABLED)
  # -O1 is suggested in clang docs to get reasonable performance
  target_compile_options (${name} PRIVATE -O1 -fno-omit-frame-pointer -fsanitize=${sanitize} -fno-sanitize-recover=all)
  set_target_properties (${name} PROPERTIES LINK_FLAGS "-fsanitize=${sanitize}")
endif ()

add_test (
  NAME testModules
  COMMAND testModules -o ${PROJECT_BINARY_DIR}/test_outputs/junit-modules.xml -s -r junit)
