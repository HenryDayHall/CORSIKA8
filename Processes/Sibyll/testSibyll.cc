#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <fwk/ParticleProperties.h>
#include <fwk/PhysicalUnits.h>
#include <process/sibyll/ParticleConversion.h>

using namespace process::sibyll;

TEST_CASE("Sibyll", "[processes]") {

  SECTION("ParticleConversion") {
    REQUIRE(fwk::particle::Electron::GetCode() == Sibyll2Corsika.at(PID::E_MINUS));
  }

  SECTION("Data") {
    REQUIRE(fwk::particle::GetName(Sibyll2Corsika.at(PID::E_PLUS)) == "e+");
  }

  SECTION("bla") {}

  SECTION("blubb") {}
}
