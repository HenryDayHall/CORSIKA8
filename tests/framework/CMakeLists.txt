
set (test_framework_sources  
  TestMain.cpp
  testFourVector.cpp
  testSaveBoostHistogram.cpp
  #   testSwitchProcessSequence.cpp this does only test the SwitchProcess -> removed
  testClassTimer.cpp
  testLogging.cpp
  testParticles.cpp
  testStackInterface.cpp
  testProcessSequence.cpp
  testCOMBoost.cpp
  #testCorsikaFenv.cpp # does not work because of use of exceptions in catch2
  testFunctionTimer.cpp
  testSecondaryView.cpp
  testGeometry.cpp
  testCombinedStack.cpp
  testUnits.cpp
  testCascade.cpp 
  testRandom.cpp
  testNullModel.cpp  
  testHelix.cpp
  )

CORSIKA_ADD_TEST (testFramework SOURCES ${test_framework_sources})
