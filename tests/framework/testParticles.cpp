/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <catch2/catch.hpp>

using namespace corsika::units;
using namespace corsika::units::si;
using namespace corsika;

TEST_CASE("ParticleProperties", "[Particles]") {

  SECTION("Types") {
    CHECK(Electron::GetCode() == Code::Electron);
    CHECK(Positron::GetCode() == Code::Positron);
    CHECK(Proton::GetCode() == Code::Proton);
    CHECK(Neutron::GetCode() == Code::Neutron);
    CHECK(Gamma::GetCode() == Code::Gamma);
    CHECK(PiPlus::GetCode() == Code::PiPlus);
  }

  SECTION("Masses") {
    CHECK(Electron::GetMass() / (511_keV) == Approx(1));
    CHECK(Electron::GetMass() / GetMass(Code::Electron) == Approx(1));

    CHECK((Proton::GetMass() + Neutron::GetMass()) /
              corsika::units::constants::nucleonMass ==
          Approx(2));
  }

  SECTION("Charges") {
    CHECK(Electron::GetCharge() / constants::e == Approx(-1));
    CHECK(Positron::GetCharge() / constants::e == Approx(+1));
    CHECK(GetCharge(Positron::GetAntiParticle()) / constants::e == Approx(-1));
  }

  SECTION("Names") {
    CHECK(Electron::GetName() == "e-");
    CHECK(PiMinus::GetName() == "pi-");
    CHECK(Nucleus::GetName() == "nucleus");
    CHECK(Iron::GetName() == "iron");
  }

  SECTION("PDG") {
    CHECK(GetPDG(Code::PiPlus) == PDGCode::PiPlus);
    CHECK(GetPDG(Code::DPlus) == PDGCode::DPlus);
    CHECK(GetPDG(Code::NuMu) == PDGCode::NuMu);
    CHECK(GetPDG(Code::NuE) == PDGCode::NuE);
    CHECK(GetPDG(Code::MuMinus) == PDGCode::MuMinus);

    CHECK(static_cast<int>(GetPDG(Code::PiPlus)) == 211);
    CHECK(static_cast<int>(GetPDG(Code::DPlus)) == 411);
    CHECK(static_cast<int>(GetPDG(Code::NuMu)) == 14);
    CHECK(static_cast<int>(GetPDG(Code::NuEBar)) == -12);
    CHECK(static_cast<int>(GetPDG(Code::MuMinus)) == 13);
  }

  SECTION("Conversion PDG -> internal") {
    CHECK(ConvertFromPDG(PDGCode::KStarMinus) == Code::KStarMinus);
    CHECK(ConvertFromPDG(PDGCode::MuPlus) == Code::MuPlus);
    CHECK(ConvertFromPDG(PDGCode::SigmaStarCMinusBar) == Code::SigmaStarCMinusBar);
  }

  SECTION("Lifetimes") {
    CHECK(GetLifetime(Code::Electron) ==
          std::numeric_limits<double>::infinity() * corsika::units::si::second);
    CHECK(GetLifetime(Code::DPlus) < GetLifetime(Code::Gamma));
    CHECK(GetLifetime(Code::RhoPlus) / corsika::units::si::second ==
          (Approx(4.414566727909413e-24).epsilon(1e-3)));
    CHECK(GetLifetime(Code::SigmaMinusBar) / corsika::units::si::second ==
          (Approx(8.018880848563575e-11).epsilon(1e-5)));
    CHECK(GetLifetime(Code::MuPlus) / corsika::units::si::second ==
          (Approx(2.1970332555864364e-06).epsilon(1e-5)));
  }

  SECTION("Particle groups: electromagnetic") {
    CHECK(IsEM(Code::Gamma));
    CHECK(IsEM(Code::Electron));
    CHECK_FALSE(IsEM(Code::MuPlus));
    CHECK_FALSE(IsEM(Code::NuE));
    CHECK_FALSE(IsEM(Code::Proton));
    CHECK_FALSE(IsEM(Code::PiPlus));
    CHECK_FALSE(IsEM(Code::Oxygen));
  }

  SECTION("Particle groups: hadrons") {
    CHECK_FALSE(IsHadron(Code::Gamma));
    CHECK_FALSE(IsHadron(Code::Electron));
    CHECK_FALSE(IsHadron(Code::MuPlus));
    CHECK_FALSE(IsHadron(Code::NuE));
    CHECK(IsHadron(Code::Proton));
    CHECK(IsHadron(Code::PiPlus));
    CHECK(IsHadron(Code::Oxygen));
    CHECK(IsHadron(Code::Nucleus));
  }

  SECTION("Particle groups: muons") {
    CHECK_FALSE(IsMuon(Code::Gamma));
    CHECK_FALSE(IsMuon(Code::Electron));
    CHECK(IsMuon(Code::MuPlus));
    CHECK_FALSE(IsMuon(Code::NuE));
    CHECK_FALSE(IsMuon(Code::Proton));
    CHECK_FALSE(IsMuon(Code::PiPlus));
    CHECK_FALSE(IsMuon(Code::Oxygen));
  }

  SECTION("Particle groups: neutrinos") {
    CHECK_FALSE(IsNeutrino(Code::Gamma));
    CHECK_FALSE(IsNeutrino(Code::Electron));
    CHECK_FALSE(IsNeutrino(Code::MuPlus));
    CHECK(IsNeutrino(Code::NuE));
    CHECK_FALSE(IsNeutrino(Code::Proton));
    CHECK_FALSE(IsNeutrino(Code::PiPlus));
    CHECK_FALSE(IsNeutrino(Code::Oxygen));
  }

  SECTION("Nuclei") {
    CHECK_FALSE(IsNucleus(Code::Gamma));
    CHECK(IsNucleus(Code::Argon));
    CHECK_FALSE(IsNucleus(Code::Proton));
    CHECK(IsNucleus(Code::Hydrogen));
    CHECK(Argon::IsNucleus());
    CHECK_FALSE(EtaC::IsNucleus());

    CHECK(GetNucleusA(Code::Hydrogen) == 1);
    CHECK(GetNucleusA(Code::Tritium) == 3);
    CHECK(Hydrogen::GetNucleusZ() == 1);
    CHECK(Tritium::GetNucleusA() == 3);

    // Nucleus is a generic object, it has no specific properties
    CHECK_THROWS(GetNucleusA(Code::Nucleus));
    CHECK_THROWS(GetNucleusZ(Code::Nucleus));
    CHECK_THROWS(GetMass(Code::Nucleus));
    CHECK_THROWS(GetCharge(Code::Nucleus));
  }
}
