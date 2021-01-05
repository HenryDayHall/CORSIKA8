set (test_stack_sources
  TestMain.cpp
  testHistoryStack.cpp
  testHistoryView.cpp
  testGeometryNodeStackExtension.cpp
  testDummyStack.cpp
  testSimpleStack.cpp
  testNuclearStackExtension.cpp
  )

CORSIKA_ADD_TEST (testStack SOURCES ${test_stack_sources})
