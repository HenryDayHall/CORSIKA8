/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/**
   @file Particles.h

   Interface to particle properties
 */

#ifndef _include_corsika_particles_ParticleProperties_h_
#define _include_corsika_particles_ParticleProperties_h_

#include <array>
#include <cstdint>
#include <iosfwd>
#include <type_traits>

#include <corsika/units/PhysicalUnits.h>

/**
 *
 * The properties of all elementary particles is stored here. The data
 * are taken from the Pythia ParticleData.xml file.
 *
 */

namespace corsika::particles {

  /**
   * @enum Code
   * The Code enum is the actual place to define CORSIKA 8 particle codes.
   */
  enum class Code : int16_t;
  enum class PDGCode : int32_t;
  using CodeIntType = std::underlying_type<Code>::type;
  using PDGCodeType = std::underlying_type<PDGCode>::type;

  // forward declarations to be used in GeneratedParticleProperties
  int16_t constexpr GetChargeNumber(Code const);
  corsika::units::si::ElectricChargeType constexpr GetCharge(Code const);
  corsika::units::si::HEPMassType constexpr GetMass(Code const);
  PDGCode constexpr GetPDG(Code const);
  constexpr std::string const& GetName(Code const);
  corsika::units::si::TimeType constexpr GetLifetime(Code const);

  bool constexpr IsNucleus(Code const);
  bool constexpr IsHadron(Code const);
  bool constexpr IsEM(Code const);
  bool constexpr IsMuon(Code const);
  bool constexpr IsNeutrino(Code const);
  int constexpr GetNucleusA(Code const);
  int constexpr GetNucleusZ(Code const);

#include <corsika/particles/GeneratedParticleProperties.inc>

  /*!
   * returns mass of particle in natural units
   */
  corsika::units::si::HEPMassType constexpr GetMass(Code const p) {
    if (p == Code::Nucleus)
      throw std::runtime_error("Cannot GetMass() of particle::Nucleus -> unspecified");
    return detail::masses[static_cast<CodeIntType>(p)];
  }

  /*!
   * returns PDG id
   */
  PDGCode constexpr GetPDG(Code const p) {
    return detail::pdg_codes[static_cast<CodeIntType>(p)];
  }

  /*!
   * returns electric charge number of particle return 1 for a proton.
   */
  int16_t constexpr GetChargeNumber(Code const p) {
    if (p == Code::Nucleus)
      throw std::runtime_error(
          "Cannot GetChargeNumber() of particle::Nucleus -> unspecified");
    // electric_charges stores charges in units of (e/3), e.g. 3 for a proton
    return detail::electric_charges[static_cast<CodeIntType>(p)] / 3;
  }

  /*!
   * returns electric charge of particle, e.g. return 1.602e-19_C for a proton.
   */
  corsika::units::si::ElectricChargeType constexpr GetCharge(Code const p) {
    if (p == Code::Nucleus)
      throw std::runtime_error("Cannot GetCharge() of particle::Nucleus -> unspecified");
    return GetChargeNumber(p) * corsika::units::constants::e;
  }

  constexpr std::string const& GetName(Code const p) {
    return detail::names[static_cast<CodeIntType>(p)];
  }

  corsika::units::si::TimeType constexpr GetLifetime(Code const p) {
    return detail::lifetime[static_cast<CodeIntType>(p)] * corsika::units::si::second;
  }

  bool constexpr IsHadron(Code const p) {
    return detail::isHadron[static_cast<CodeIntType>(p)];
  }

  bool constexpr IsEM(Code c) {
    return c == Code::Electron || c == Code::Positron || c == Code::Gamma;
  }

  bool constexpr IsMuon(Code c) { return c == Code::MuPlus || c == Code::MuMinus; }

  bool constexpr IsNeutrino(Code c) {
    return c == Code::NuE || c == Code::NuMu || c == Code::NuTau || c == Code::NuEBar ||
           c == Code::NuMuBar || c == Code::NuTauBar;
  }

  int constexpr GetNucleusA(Code const p) {
    if (p == Code::Nucleus) {
      throw std::runtime_error("GetNucleusA(Code::Nucleus) is impossible!");
    }
    return detail::nucleusA[static_cast<CodeIntType>(p)];
  }

  int constexpr GetNucleusZ(Code const p) {
    if (p == Code::Nucleus) {
      throw std::runtime_error("GetNucleusZ(Code::Nucleus) is impossible!");
    }
    return detail::nucleusZ[static_cast<CodeIntType>(p)];
  }

  bool constexpr IsNucleus(Code const p) {
    return (p == Code::Nucleus) || (GetNucleusA(p) != 0);
  }

  /**
   * the output operator for humand-readable particle codes
   **/

  std::ostream& operator<<(std::ostream&, corsika::particles::Code);

  Code ConvertFromPDG(PDGCode);

  /**
   * Get mass of nucleus
   **/
  corsika::units::si::HEPMassType constexpr GetNucleusMass(const int vA, const int vZ) {
    return Proton::GetMass() * vZ + (vA - vZ) * Neutron::GetMass();
  }

} // namespace corsika::particles

#endif
