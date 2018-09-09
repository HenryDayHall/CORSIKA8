#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

#include <fwk/PhysicalUnits.h>

#include <fwk/ParticleProperties.h>

using namespace fwk::literals;
using namespace fwk::particle;

TEST_CASE("Particles", "[Particles]") {

  SECTION("Types") { REQUIRE(Electron::GetCode() == Code::Electron); }

  SECTION("Data") {
    REQUIRE(Electron::GetMass() / 0.511_MeV == Approx(1));
    REQUIRE(Electron::GetMass() / GetMass(Code::Electron) == Approx(1));
    REQUIRE(Electron::GetCharge() / fwk::constants::e == Approx(-1));
    REQUIRE(Positron::GetCharge() / fwk::constants::e == Approx(+1));
    REQUIRE(GetElectricCharge(Positron::GetAntiParticle()) / fwk::constants::e ==
            Approx(-1));
    REQUIRE(Electron::GetName() == "e-");
  }
}
