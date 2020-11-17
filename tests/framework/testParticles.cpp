/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("ParticleProperties", "[Particles]") {

  SECTION("Types") {
    REQUIRE(Electron::code == Code::Electron);
    REQUIRE(Positron::code == Code::Positron);
    REQUIRE(Proton::code == Code::Proton);
    REQUIRE(Neutron::code == Code::Neutron);
    REQUIRE(Gamma::code == Code::Gamma);
    REQUIRE(PiPlus::code == Code::PiPlus);
  }

  SECTION("Masses") {
    REQUIRE(Electron::mass() / (511_keV) == Approx(1));
    REQUIRE(Electron::mass() / mass(Code::Electron) == Approx(1));

    REQUIRE((Proton::mass() + Neutron::mass()) / constants::nucleonMass == Approx(2));
  }

  SECTION("Charges") {
    REQUIRE(Electron::charge() / constants::e == Approx(-1));
    REQUIRE(Positron::charge() / constants::e == Approx(+1));
    REQUIRE(charge(Positron::anti_code) / constants::e == Approx(-1));
  }

  SECTION("Names") {
    REQUIRE(Electron::name() == "e-");
    REQUIRE(PiMinus::name() == "pi-");
    REQUIRE(Nucleus::name() == "nucleus");
    REQUIRE(Iron::name() == "iron");
  }

  SECTION("PDG") {
    REQUIRE(PDG(Code::PiPlus) == PDGCode::PiPlus);
    REQUIRE(PDG(Code::DPlus) == PDGCode::DPlus);
    REQUIRE(PDG(Code::NuMu) == PDGCode::NuMu);
    REQUIRE(PDG(Code::NuE) == PDGCode::NuE);
    REQUIRE(PDG(Code::MuMinus) == PDGCode::MuMinus);

    REQUIRE(static_cast<int>(PDG(Code::PiPlus)) == 211);
    REQUIRE(static_cast<int>(PDG(Code::DPlus)) == 411);
    REQUIRE(static_cast<int>(PDG(Code::NuMu)) == 14);
    REQUIRE(static_cast<int>(PDG(Code::NuEBar)) == -12);
    REQUIRE(static_cast<int>(PDG(Code::MuMinus)) == 13);
  }

  SECTION("Conversion PDG -> internal") {
    REQUIRE(convert_from_PDG(PDGCode::KStarMinus) == Code::KStarMinus);
    REQUIRE(convert_from_PDG(PDGCode::MuPlus) == Code::MuPlus);
    REQUIRE(convert_from_PDG(PDGCode::SigmaStarCMinusBar) == Code::SigmaStarCMinusBar);
  }

  SECTION("Lifetimes") {
    REQUIRE(lifetime(Code::Electron) ==
            std::numeric_limits<double>::infinity() * si::second);
    REQUIRE(lifetime(Code::DPlus) < lifetime(Code::Gamma));
    REQUIRE(lifetime(Code::RhoPlus) / si::second ==
            (Approx(4.414566727909413e-24).epsilon(1e-3)));
    REQUIRE(lifetime(Code::SigmaMinusBar) / si::second ==
            (Approx(8.018880848563575e-11).epsilon(1e-5)));
    REQUIRE(lifetime(Code::MuPlus) / si::second ==
            (Approx(2.1970332555864364e-06).epsilon(1e-5)));
  }

  SECTION("Particle groups: electromagnetic") {
    REQUIRE(is_em(Code::Gamma));
    REQUIRE(is_em(Code::Electron));
    REQUIRE_FALSE(is_em(Code::MuPlus));
    REQUIRE_FALSE(is_em(Code::NuE));
    REQUIRE_FALSE(is_em(Code::Proton));
    REQUIRE_FALSE(is_em(Code::PiPlus));
    REQUIRE_FALSE(is_em(Code::Oxygen));
  }

  SECTION("Particle groups: hadrons") {
    REQUIRE_FALSE(is_hadron(Code::Gamma));
    REQUIRE_FALSE(is_hadron(Code::Electron));
    REQUIRE_FALSE(is_hadron(Code::MuPlus));
    REQUIRE_FALSE(is_hadron(Code::NuE));
    REQUIRE(is_hadron(Code::Proton));
    REQUIRE(is_hadron(Code::PiPlus));
    REQUIRE(is_hadron(Code::Oxygen));
    REQUIRE(is_hadron(Code::Nucleus));
  }

  SECTION("Particle groups: muons") {
    REQUIRE_FALSE(is_muon(Code::Gamma));
    REQUIRE_FALSE(is_muon(Code::Electron));
    REQUIRE(is_muon(Code::MuPlus));
    REQUIRE(is_muon(Code::MuMinus));
    REQUIRE_FALSE(is_muon(Code::NuE));
    REQUIRE_FALSE(is_muon(Code::Proton));
    REQUIRE_FALSE(is_muon(Code::PiPlus));
    REQUIRE_FALSE(is_muon(Code::Oxygen));
  }

  SECTION("Particle groups: neutrinos") {
    REQUIRE_FALSE(is_neutrino(Code::Gamma));
    REQUIRE_FALSE(is_neutrino(Code::Electron));
    REQUIRE_FALSE(is_neutrino(Code::MuPlus));
    REQUIRE_FALSE(is_neutrino(Code::Proton));
    REQUIRE_FALSE(is_neutrino(Code::PiPlus));
    REQUIRE_FALSE(is_neutrino(Code::Oxygen));

    REQUIRE(is_neutrino(Code::NuE));
    REQUIRE(is_neutrino(Code::NuMu));
    REQUIRE(is_neutrino(Code::NuTau));
    REQUIRE(is_neutrino(Code::NuEBar));
    REQUIRE(is_neutrino(Code::NuMuBar));
    REQUIRE(is_neutrino(Code::NuTauBar));
  }

  SECTION("Nuclei") {
    REQUIRE_FALSE(is_nucleus(Code::Gamma));
    REQUIRE(is_nucleus(Code::Argon));
    REQUIRE_FALSE(is_nucleus(Code::Proton));
    REQUIRE(is_nucleus(Code::Hydrogen));
    REQUIRE(Argon::is_nucleus());
    REQUIRE_FALSE(EtaC::is_nucleus());

    REQUIRE(nucleus_A(Code::Hydrogen) == 1);
    REQUIRE(nucleus_A(Code::Tritium) == 3);
    REQUIRE(Hydrogen::nucleus_Z() == 1);
    REQUIRE(Tritium::nucleus_A() == 3);

    REQUIRE_THROWS(nucleus_Z(Code::Nucleus));
    REQUIRE_THROWS(nucleus_A(Code::Nucleus));
  }
}
