set (test_modules_sources
  TestMain.cpp
  testStackInspector.cpp
  testTracking.cpp
  ## testExecTime.cpp --> needs to be fixed, see #326
  testObservationPlane.cpp
  testQGSJetII.cpp
  testProposal.cpp
  testPythia8.cpp
  testUrQMD.cpp
  testCONEX.cpp
  testParticleCut.cpp
  testSibyll.cpp
  testEpos.cpp
  testRadio.cpp
  testEMThinning.cpp
  testSophia.cpp
)
  
if (DEFINED C8_FLUKALIB)
    list(APPEND test_modules_sources "testFluka.cpp")
endif()

CORSIKA_ADD_TEST (testModules SOURCES ${test_modules_sources})

target_compile_definitions (
  testModules
  PRIVATE
  REFDATADIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
