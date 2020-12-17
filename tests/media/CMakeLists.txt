set (test_media_sources
  TestMain.cpp
  testMedium.cpp
  testEnvironment.cpp
  testRefractiveIndex.cpp
  testShowerAxis.cpp
  )

CORSIKA_ADD_TEST (testMedia SOURCES ${test_media_sources})

target_link_libraries (testMedia CONAN_PKG::boost) # for math/quadrature/gauss_kronrod.hpp
