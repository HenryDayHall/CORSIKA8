
set (test_framework_sources  
  # testCascade.cpp this is most important, but whole content of former Processes folder missing yet
  testClassTimer.cpp
  #testCombinedStack.cpp #FIXME: reenable this
  #testCOMBoost.cpp #FIXME: reenable this
  #testCorsikaFenv.cpp # does not work because of use of exceptions in catch2
  testFourVector.cpp
  testFunctionTimer.cpp
  testGeometry.cpp
  testLogging.cpp
  TestMain.cpp
  testParticles.cpp
  testProcessSequence.cpp
  testRandom.cpp
  #testSecondaryView.cpp #FIXME: reenable this
  #testStackInterface.cpp #FIXME: reenable this
  testUnits.cpp
  )

CORSIKA_ADD_TEST (testFramework SOURCES ${test_framework_sources})
