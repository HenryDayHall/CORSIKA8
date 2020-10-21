set (test_media_sources
  TestMain.cpp
  testEnvironment.cpp
  )

CORSIKA_ADD_TEST (testMedia SOURCES ${test_media_sources})
