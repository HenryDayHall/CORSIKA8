
set (test_framework_sources  
  # testCascade.cpp this is most important, but whole content of former Processes folder missing yet
  testClassTimer.cpp
  testCombinedStack.cpp
  testCOMBoost.cpp
  #testCorsikaFenv.cpp # does not work because of use of exceptions in catch2
  testFourVector.cpp
  testHelix.cpp
  testFunctionTimer.cpp
  testGeometry.cpp
  testLogging.cpp
  TestMain.cpp
  testParticles.cpp
  testNullModel.cpp
  testProcessSequence.cpp
  testRandom.cpp
  testSecondaryView.cpp
  testStackInterface.cpp
  testUnits.cpp
  )

CORSIKA_ADD_TEST (testFramework SOURCES ${test_framework_sources})
