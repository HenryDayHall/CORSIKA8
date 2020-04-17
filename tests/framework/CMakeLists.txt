
set (sources
  # testCascade.cpp this is most important, but whole content of former Processes folder missing yet
  testCombinedStack.cpp
  testCOMBoost.cpp
  #testCorsikaFenv.cpp # does not work because of use of exceptions in catch2
  testFourVector.cpp
  testGeometry.cpp
  #testLogging.cpp # irrelevant -> not used back in March
  TestMain.cpp
  testParticles.cpp
  testProcessSequence.cpp
  testRandom.cpp
  testSecondaryView.cpp
  testStackInterface.cpp
  testUnits.cpp
  )


add_executable (testFramework ${sources})

target_link_libraries (testFramework CORSIKA8 Catch2)

target_compile_options (testFramework PRIVATE -g) # do not skip asserts

target_include_directories (testFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/test_outputs/)

if (CORSIKA_SANITIZERS_ENABLED)
  # -O1 is suggested in clang docs to get reasonable performance
  target_compile_options (${name} PRIVATE -O1 -fno-omit-frame-pointer -fsanitize=${sanitize} -fno-sanitize-recover=all)
  set_target_properties (${name} PROPERTIES LINK_FLAGS "-fsanitize=${sanitize}")
endif ()

add_test (
  NAME testFramework
  COMMAND testFramework -o ${PROJECT_BINARY_DIR}/test_outputs/junit-${name}.xml -s -r junit)
