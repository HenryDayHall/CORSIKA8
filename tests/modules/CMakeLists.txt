set (test_modules_sources
  TestMain.cpp
  testSibyll.cpp
  testNullModel.cpp
  testObservationPlane.cpp
  testParticleCut.cpp
  testPythia8.cpp
  testQGSJetII.cpp
  testStackInspector.cpp
  testSwitchProcess.cpp
  testTrackingLine.cpp
  testUrQMD.cpp
  )

CORSIKA_ADD_TEST (testModules SOURCES ${test_modules_sources})

