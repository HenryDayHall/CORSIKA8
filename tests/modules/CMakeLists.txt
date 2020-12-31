set (test_modules_sources
  TestMain.cpp
  testStackInspector.cpp
  testTracking.cpp
  # testExecTime.cpp
  testObservationPlane.cpp
  testQGSJetII.cpp
  #    testSwitchProcess.cpp  -> gone
  testPythia8.cpp
  testUrQMD.cpp
  testCONEX.cpp
  testOnShellCheck.cpp
  testParticleCut.cpp
  testSibyll.cpp
  #  testNullModel.cpp  -> gone, framework now
  )

CORSIKA_ADD_TEST (testModules SOURCES ${test_modules_sources})

target_compile_definitions (
  testModules
  PRIVATE
  REFDATADIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
