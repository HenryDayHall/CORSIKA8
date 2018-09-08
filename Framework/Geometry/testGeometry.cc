#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <fwk/PhysicalUnits.h>

using namespace phys::units;
using namespace phys::units::literals;

TEST_CASE("Geometry", "[Geometry]") {
  SECTION("Coordinate Systems") {}

  SECTION("Point") {}

  SECTION("Vector") {}

  SECTION("Helix") {}

  SECTION("Line") {}
}
