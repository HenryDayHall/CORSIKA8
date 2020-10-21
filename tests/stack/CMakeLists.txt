set (test_stack_sources
  TestMain.cpp
  testSuperStupidStack.cpp
  testNuclearStackExtension.cpp
  testGeometryNodeStackExtension.cpp
  testDummyStack.cpp
  )

CORSIKA_ADD_TEST (testStack SOURCES ${test_stack_sources})
