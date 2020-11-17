set (test_media_sources
  TestMain.cpp
  testMedium.cpp
  testEnvironment.cpp
  testRefractiveIndex.cpp
  )

CORSIKA_ADD_TEST (testMedia SOURCES ${test_media_sources})
