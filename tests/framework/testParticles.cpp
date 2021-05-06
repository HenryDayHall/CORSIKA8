/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("ParticleProperties", "[Particles]") {

  logging::set_level(logging::level::info);

  SECTION("Types") {
    CHECK(Electron::code == Code::Electron);
    CHECK(Positron::code == Code::Positron);
    CHECK(Proton::code == Code::Proton);
    CHECK(Neutron::code == Code::Neutron);
    CHECK(Photon::code == Code::Photon);
    CHECK(PiPlus::code == Code::PiPlus);
  }

  SECTION("Masses") {
    CHECK(Electron::mass / (511_keV) == Approx(1));
    CHECK(Electron::mass / get_mass(Code::Electron) == 1.);
    CHECK(Photon::mass / (1_eV) == 0.);
    CHECK(Photon::mass == get_mass(Code::Photon));

    CHECK((Proton::mass + Neutron::mass) / constants::nucleonMass == Approx(2));
  }

  SECTION("Charges") {
    CHECK(Electron::charge / constants::e == Approx(-1));
    CHECK(Positron::charge / constants::e == Approx(+1));
    CHECK(get_charge(Positron::anti_code) / constants::e == Approx(-1));
    CHECK(Photon::charge / constants::e == 0.);
  }

  SECTION("Names") {
    CHECK(Electron::name == "e-");
    CHECK(get_name(Code::Electron) == "e-");
    CHECK(PiMinus::name == "pi-");
    CHECK(Iron::name == "iron");
    CHECK(Photon::name == "photon");
  }

  SECTION("PDG") {
    CHECK(get_PDG(Code::PiPlus) == PDGCode::PiPlus);
    CHECK(get_PDG(Code::DPlus) == PDGCode::DPlus);
    CHECK(get_PDG(Code::NuMu) == PDGCode::NuMu);
    CHECK(get_PDG(Code::NuE) == PDGCode::NuE);
    CHECK(get_PDG(Code::MuMinus) == PDGCode::MuMinus);
    CHECK(get_PDG(Code::Photon) == PDGCode::Photon);

    CHECK(static_cast<int>(get_PDG(Code::PiPlus)) == 211);
    CHECK(static_cast<int>(get_PDG(Code::DPlus)) == 411);
    CHECK(static_cast<int>(get_PDG(Code::NuMu)) == 14);
    CHECK(static_cast<int>(get_PDG(Code::NuEBar)) == -12);
    CHECK(static_cast<int>(get_PDG(Code::MuMinus)) == 13);
    CHECK(static_cast<int>(get_PDG(Code::Photon)) == 22);
  }

  SECTION("Conversion PDG -> internal") {
    CHECK(convert_from_PDG(PDGCode::KStarMinus) == Code::KStarMinus);
    CHECK(convert_from_PDG(PDGCode::MuPlus) == Code::MuPlus);
    CHECK(convert_from_PDG(PDGCode::SigmaStarCMinusBar) == Code::SigmaStarCMinusBar);
  }

  SECTION("Lifetimes") {
    CHECK(get_lifetime(Code::Electron) ==
          std::numeric_limits<double>::infinity() * si::second);
    CHECK(get_lifetime(Code::DPlus) < get_lifetime(Code::Photon));
    CHECK(get_lifetime(Code::RhoPlus) / si::second ==
          (Approx(4.414566727909413e-24).epsilon(1e-3)));
    CHECK(get_lifetime(Code::SigmaMinusBar) / si::second ==
          (Approx(8.018880848563575e-11).epsilon(1e-5)));
    CHECK(get_lifetime(Code::MuPlus) / si::second ==
          (Approx(2.1970332555864364e-06).epsilon(1e-5)));
  }

  SECTION("Energy threshold") {
    //! by default energy thresholds are set to particle mass
    CHECK(get_energy_threshold(Electron::code) / Electron::mass == Approx(1));

    set_energy_threshold(Electron::code, 10_GeV);
    CHECK_FALSE(get_energy_threshold(Code::Electron) == 1_GeV);
    CHECK(get_energy_threshold(Code::Electron) == 10_GeV);
  }

  SECTION("Particle groups: electromagnetic") {
    CHECK(is_em(Code::Photon));
    CHECK(is_em(Code::Electron));
    CHECK_FALSE(is_em(Code::MuPlus));
    CHECK_FALSE(is_em(Code::NuE));
    CHECK_FALSE(is_em(Code::Proton));
    CHECK_FALSE(is_em(Code::PiPlus));
    CHECK_FALSE(is_em(Code::Oxygen));
  }

  SECTION("Particle groups: hadrons") {
    CHECK_FALSE(is_hadron(Code::Photon));
    CHECK_FALSE(is_hadron(Code::Electron));
    CHECK_FALSE(is_hadron(Code::MuPlus));
    CHECK_FALSE(is_hadron(Code::NuE));
    CHECK(is_hadron(Code::Proton));
    CHECK(is_hadron(Code::PiPlus));
    CHECK(is_hadron(Code::Oxygen));
    CHECK(is_hadron(Code::Nucleus));
  }

  SECTION("Particle groups: muons") {
    CHECK_FALSE(is_muon(Code::Photon));
    CHECK_FALSE(is_muon(Code::Electron));
    CHECK(is_muon(Code::MuPlus));
    CHECK(is_muon(Code::MuMinus));
    CHECK_FALSE(is_muon(Code::NuE));
    CHECK_FALSE(is_muon(Code::Proton));
    CHECK_FALSE(is_muon(Code::PiPlus));
    CHECK_FALSE(is_muon(Code::Oxygen));
  }

  SECTION("Particle groups: neutrinos") {
    CHECK_FALSE(is_neutrino(Code::Photon));
    CHECK_FALSE(is_neutrino(Code::Electron));
    CHECK_FALSE(is_neutrino(Code::MuPlus));
    CHECK_FALSE(is_neutrino(Code::Proton));
    CHECK_FALSE(is_neutrino(Code::PiPlus));
    CHECK_FALSE(is_neutrino(Code::Oxygen));

    CHECK(is_neutrino(Code::NuE));
    CHECK(is_neutrino(Code::NuMu));
    CHECK(is_neutrino(Code::NuTau));
    CHECK(is_neutrino(Code::NuEBar));
    CHECK(is_neutrino(Code::NuMuBar));
    CHECK(is_neutrino(Code::NuTauBar));
  }

  SECTION("Nuclei") {
    CHECK_FALSE(is_nucleus(Code::Photon));
    CHECK(is_nucleus(Code::Argon));
    CHECK_FALSE(is_nucleus(Code::Proton));
    CHECK(is_nucleus(Code::Hydrogen));
    CHECK(Argon::is_nucleus);
    CHECK_FALSE(EtaC::is_nucleus);

    CHECK(get_nucleus_A(Code::Hydrogen) == 1);
    CHECK(get_nucleus_A(Code::Tritium) == 3);
    CHECK(Hydrogen::nucleus_Z == 1);
    CHECK(Tritium::nucleus_A == 3);

    // Nucleus is a generic object, it has no specific properties
    CHECK_THROWS(get_nucleus_Z(Code::Nucleus));
    CHECK_THROWS(get_nucleus_A(Code::Nucleus));
    CHECK_THROWS(get_mass(Code::Nucleus));
    CHECK_THROWS(get_charge(Code::Nucleus));
  }
}
