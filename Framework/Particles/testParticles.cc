/*
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

    REQUIRE((Proton::GetMass() + Neutron::GetMass()) /
                corsika::units::constants::nucleonMass ==
            Approx(2));
  }

  SECTION("Charges") {
    REQUIRE(Electron::GetCharge() / constants::e == Approx(-1));
    REQUIRE(Positron::GetCharge() / constants::e == Approx(+1));
    REQUIRE(GetCharge(Positron::GetAntiParticle()) / constants::e == Approx(-1));
  }

  SECTION("Names") {
    REQUIRE(Electron::GetName() == "e-");
    REQUIRE(PiMinus::GetName() == "pi-");
    REQUIRE(Nucleus::GetName() == "nucleus");
    REQUIRE(Iron::GetName() == "iron");
  }

  SECTION("PDG") {
    REQUIRE(GetPDG(Code::PiPlus) == PDGCode::PiPlus);
    REQUIRE(GetPDG(Code::DPlus) == PDGCode::DPlus);
    REQUIRE(GetPDG(Code::NuMu) == PDGCode::NuMu);
    REQUIRE(GetPDG(Code::NuE) == PDGCode::NuE);
    REQUIRE(GetPDG(Code::MuMinus) == PDGCode::MuMinus);

    REQUIRE(static_cast<int>(GetPDG(Code::PiPlus)) == 211);
    REQUIRE(static_cast<int>(GetPDG(Code::DPlus)) == 411);
    REQUIRE(static_cast<int>(GetPDG(Code::NuMu)) == 14);
    REQUIRE(static_cast<int>(GetPDG(Code::NuEBar)) == -12);
    REQUIRE(static_cast<int>(GetPDG(Code::MuMinus)) == 13);
  }

  SECTION("Conversion PDG -> internal") {
    REQUIRE(ConvertFromPDG(PDGCode::KStarMinus) == Code::KStarMinus);
    REQUIRE(ConvertFromPDG(PDGCode::MuPlus) == Code::MuPlus);
    REQUIRE(ConvertFromPDG(PDGCode::SigmaStarCMinusBar) == Code::SigmaStarCMinusBar);
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

  SECTION("Particle groups: electromagnetic") {
    REQUIRE(IsEM(Code::Gamma));
    REQUIRE(IsEM(Code::Electron));
    REQUIRE_FALSE(IsEM(Code::MuPlus));
    REQUIRE_FALSE(IsEM(Code::NuE));
    REQUIRE_FALSE(IsEM(Code::Proton));
    REQUIRE_FALSE(IsEM(Code::PiPlus));
    REQUIRE_FALSE(IsEM(Code::Oxygen));
  }

  SECTION("Particle groups: hadrons") {
    REQUIRE_FALSE(IsHadron(Code::Gamma));
    REQUIRE_FALSE(IsHadron(Code::Electron));
    REQUIRE_FALSE(IsHadron(Code::MuPlus));
    REQUIRE_FALSE(IsHadron(Code::NuE));
    REQUIRE(IsHadron(Code::Proton));
    REQUIRE(IsHadron(Code::PiPlus));
    REQUIRE(IsHadron(Code::Oxygen));
    REQUIRE(IsHadron(Code::Nucleus));
  }

  SECTION("Particle groups: muons") {
    REQUIRE_FALSE(IsMuon(Code::Gamma));
    REQUIRE_FALSE(IsMuon(Code::Electron));
    REQUIRE(IsMuon(Code::MuPlus));
    REQUIRE_FALSE(IsMuon(Code::NuE));
    REQUIRE_FALSE(IsMuon(Code::Proton));
    REQUIRE_FALSE(IsMuon(Code::PiPlus));
    REQUIRE_FALSE(IsMuon(Code::Oxygen));
  }

  SECTION("Particle groups: neutrinos") {
    REQUIRE_FALSE(IsNeutrino(Code::Gamma));
    REQUIRE_FALSE(IsNeutrino(Code::Electron));
    REQUIRE_FALSE(IsNeutrino(Code::MuPlus));
    REQUIRE(IsNeutrino(Code::NuE));
    REQUIRE_FALSE(IsNeutrino(Code::Proton));
    REQUIRE_FALSE(IsNeutrino(Code::PiPlus));
    REQUIRE_FALSE(IsNeutrino(Code::Oxygen));
  }

  SECTION("Nuclei") {
    REQUIRE_FALSE(IsNucleus(Code::Gamma));
    REQUIRE(IsNucleus(Code::Argon));
    REQUIRE_FALSE(IsNucleus(Code::Proton));
    REQUIRE(IsNucleus(Code::Hydrogen));
    REQUIRE(Argon::IsNucleus());
    REQUIRE_FALSE(EtaC::IsNucleus());

    REQUIRE(GetNucleusA(Code::Hydrogen) == 1);
    REQUIRE(GetNucleusA(Code::Tritium) == 3);
    REQUIRE(Hydrogen::GetNucleusZ() == 1);
    REQUIRE(Tritium::GetNucleusA() == 3);

    REQUIRE_THROWS(GetNucleusA(Code::Nucleus));
    REQUIRE_THROWS(GetNucleusZ(Code::Nucleus));
  }
}
