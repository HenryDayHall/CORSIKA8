set (test_modules_sources
  TestMain.cpp
#  ${CORSIKA_DATA}/readLib/source/testData.cc
  testStackInspector.cpp
  #testTrackingLine.cpp
  #testExecTime.cpp
  testObservationPlane.cpp
  testQGSJetII.cpp
  #    testSwitchProcess.cpp  -> gone
  #testPythia8.cpp
  #testUrQMD.cpp
  #testCONEXSourceCut.cpp
  testOnShellCheck.cpp
  testParticleCut.cpp
  testSibyll.cpp
  #  testNullModel.cpp  -> gone
  )

CORSIKA_ADD_TEST (testModules SOURCES ${test_modules_sources})
