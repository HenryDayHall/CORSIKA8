
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika::units::si;
using namespace corsika::particles;

TEST_CASE("ParticleProperties", "[Particles]") {

  SECTION("Types") {
    REQUIRE(Electron::GetCode() == Code::Electron);
    REQUIRE(Positron::GetCode() == Code::Positron);
    REQUIRE(Proton::GetCode() == Code::Proton);
    REQUIRE(Neutron::GetCode() == Code::Neutron);
    REQUIRE(Gamma::GetCode() == Code::Gamma);
    REQUIRE(PiPlus::GetCode() == Code::PiPlus);
  }

  SECTION("Masses") {
    REQUIRE(Electron::GetMass() / (511_keV / constants::cSquared) == Approx(1));
    REQUIRE(Electron::GetMass() / GetMass(Code::Electron) == Approx(1));
  }

  SECTION("Charges") {
    REQUIRE(Electron::GetCharge() / constants::e == Approx(-1));
    REQUIRE(Positron::GetCharge() / constants::e == Approx(+1));
    REQUIRE(GetElectricCharge(Positron::GetAntiParticle()) / constants::e == Approx(-1));
  }

  SECTION("Names") {
    REQUIRE(Electron::GetName() == "e-");
    REQUIRE(PiMinus::GetName() == "pi-");
  }

  SECTION("PDG") {
    REQUIRE(GetPDG(Code::PiPlus) == 211);
    REQUIRE(GetPDG(Code::DPlus) == 411); 
    REQUIRE(GetPDG(Code::NuMu) == 14); 
    REQUIRE(GetPDG(Code::NuE) == 12); 
    REQUIRE(GetPDG(Code::MuMinus) == 13); 
  }
  
}
