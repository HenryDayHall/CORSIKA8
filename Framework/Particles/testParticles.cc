
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include <catch2/catch.hpp>

using namespace corsika::units;
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
    REQUIRE(Electron::GetMass() / (511_keV) == Approx(1));
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

  SECTION("Lifetimes") {
    REQUIRE(GetLifetime(Code::Electron) ==
            std::numeric_limits<double>::infinity() * corsika::units::si::second);
    REQUIRE(GetLifetime(Code::DPlus) < GetLifetime(Code::Gamma));
    REQUIRE(GetLifetime(Code::RhoPlus) / corsika::units::si::second ==
            (Approx(4.414566727909413e-24).epsilon(1e-3)));
    REQUIRE(GetLifetime(Code::SigmaMinusBar) / corsika::units::si::second ==
            (Approx(8.018880848563575e-11).epsilon(1e-5)));
    REQUIRE(GetLifetime(Code::MuPlus) / corsika::units::si::second ==
            (Approx(2.1970332555864364e-06).epsilon(1e-5)));
  }

  SECTION("Nuclei") {
    REQUIRE(IsNucleus(Code::Gamma) == false);
    REQUIRE(IsNucleus(Code::Argon) == true);
    REQUIRE(IsNucleus(Code::Proton) == false);
    REQUIRE(IsNucleus(Code::Hydrogen) == true);
    REQUIRE(Argon::IsNucleus() == true);
    REQUIRE(EtaC::IsNucleus() == false);

    REQUIRE(GetNucleusA(Code::Hydrogen) == 1);
    REQUIRE(GetNucleusA(Code::Tritium) == 3);
    REQUIRE(Hydrogen::GetNucleusZ() == 1);
    REQUIRE(Tritium::GetNucleusA() == 3);
  }
}
