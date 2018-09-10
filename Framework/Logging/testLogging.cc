#include <corsika/logging/Logger.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

TEST_CASE("Logging", "[Logging]") {
  SECTION("sectionOne") {}

  SECTION("sectionTwo") {}

  SECTION("sectionThree") {}
}
