
set (test_framework_sources
  testBitField.cpp
  # testCascade.cpp this is most important, but whole content of former Processes folder missing yet
  testCombinedStack.cpp
  testCOMBoost.cpp
  #testCorsikaFenv.cpp # does not work because of use of exceptions in catch2
  testFourVector.cpp
  testGeometry.cpp
  testLogging.cpp
  TestMain.cpp
  testParticles.cpp
  testProcessSequence.cpp
  testRandom.cpp
  testSecondaryView.cpp
  testStackInterface.cpp
  testUnits.cpp
  )

CORSIKA_ADD_TEST (testFramework SOURCES ${test_framework_sources})
