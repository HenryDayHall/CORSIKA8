
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika::units;
using namespace corsika::particles;

TEST_CASE("Particles", "[Particles]") {

  SECTION("Types") { REQUIRE(Electron::GetCode() == Code::Electron); }

  SECTION("Data") {
    REQUIRE(Electron::GetMass() / (511_keV / constants::cSquared) == Approx(1));
    REQUIRE(Electron::GetMass() / GetMass(Code::Electron) == Approx(1));
    REQUIRE(Electron::GetCharge() / constants::e == Approx(-1));
    REQUIRE(Positron::GetCharge() / constants::e == Approx(+1));
    REQUIRE(GetElectricCharge(Positron::GetAntiParticle()) / constants::e == Approx(-1));
    REQUIRE(Electron::GetName() == "e-");
  }
}
