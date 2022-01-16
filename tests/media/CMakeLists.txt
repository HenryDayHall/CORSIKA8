set (test_media_sources
  TestMain.cpp
  testNuclearComposition.cpp
  testEnvironment.cpp
  testShowerAxis.cpp
  testMedium.cpp
  testRefractiveIndex.cpp
  testMagneticField.cpp
  testCORSIKA7Atmospheres.cpp
  )

CORSIKA_ADD_TEST (testMedia SOURCES ${test_media_sources})

target_link_libraries (testMedia CONAN_PKG::boost) # for math/quadrature/gauss_kronrod.hpp

target_compile_definitions (
  testMedia
  PRIVATE
  REFDATADIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
