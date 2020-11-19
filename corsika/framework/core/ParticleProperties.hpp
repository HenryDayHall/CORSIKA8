/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/**
   @file Particles.h

   Interface to particle properties
 */

#pragma once

#include <array>
#include <cstdint>
#include <cmath>
#include <iosfwd>

#include <corsika/framework/core/PhysicalUnits.hpp>

/**
 *
 * The properties of all elementary particles is stored here. The data
 * are taken from the Pythia ParticleData.xml file.
 *
 */

namespace corsika {
  /**
   * @enum Code
   * The Code enum is the actual place to define CORSIKA 8 particle codes.
   */
  enum class Code : int16_t;
  enum class PDGCode : int32_t;
  using CodeIntType = std::underlying_type<Code>::type;
  using PDGCodeType = std::underlying_type<PDGCode>::type;

  // forward declarations to be used in GeneratedParticleProperties
  int16_t constexpr charge_number(Code const);     //!< electric charge in units of e
  ElectricChargeType constexpr charge(Code const); //!< electric charge
  HEPMassType constexpr mass(Code const);          //!< mass

  //! Particle code according to PDG, "Monte Carlo Particle Numbering Scheme"
  PDGCode constexpr PDG(Code const);
  constexpr std::string const& name(Code const); //!< name of the particle as string
  TimeType constexpr lifetime(Code const);       //!< lifetime

  //! true iff the particle is a hard-coded nucleus or Code::Nucleus
  bool constexpr is_nucleus(Code const);
  bool constexpr is_hadron(Code const); //!< true iff particle is hadron
  bool constexpr is_em(Code const); //!< true iff particle is electron, positron or gamma
  bool constexpr is_muon(Code const);     //!< true iff particle is mu+ or mu-
  bool constexpr is_neutrino(Code const); //!< true iff particle is (anti-) neutrino
  int constexpr nucleus_A(Code const); //!< returns A for hard-coded nucleus, otherwise 0
  int constexpr nucleus_Z(Code const); //!< returns Z for hard-coded nucleus, otherwise 0
} // namespace corsika

#include <corsika/framework/core/GeneratedParticleProperties.inc>

namespace corsika {

  HEPMassType constexpr mass(Code const p) {
    if (p == Code::Nucleus)
      throw std::runtime_error("Cannot GetMass() of particle::Nucleus -> unspecified");
    return particle::detail::masses[static_cast<CodeIntType>(p)];
  }

  PDGCode constexpr PDG(Code const p) {
    return particle::detail::pdg_codes[static_cast<CodeIntType>(p)];
  }

  int16_t constexpr charge_number(Code const code) {
    if (code == Code::Nucleus)
      throw std::runtime_error("charge of particle::Nucleus undefined");
    return particle::detail::electric_charges[static_cast<CodeIntType>(code)];
  }

  ElectricChargeType constexpr charge(Code const code) {
    if (code == Code::Nucleus)
      throw std::runtime_error("Cannot GetCharge() of particle::Nucleus -> unspecified");
    return charge_number(code) * constants::e;
  }

  constexpr std::string const& name(Code const code) {
    return particle::detail::names[static_cast<CodeIntType>(code)];
  }

  TimeType constexpr lifetime(Code const p) {
    return particle::detail::lifetime[static_cast<CodeIntType>(p)] * second;
  }

  bool constexpr is_hadron(Code const p) {
    return particle::detail::isHadron[static_cast<CodeIntType>(p)];
  }

  bool constexpr is_em(Code c) {
    return c == Code::Electron || c == Code::Positron || c == Code::Gamma;
  }

  bool constexpr is_muon(Code c) { return c == Code::MuPlus || c == Code::MuMinus; }

  bool constexpr is_neutrino(Code c) {
    return c == Code::NuE || c == Code::NuMu || c == Code::NuTau || c == Code::NuEBar ||
           c == Code::NuMuBar || c == Code::NuTauBar;
  }

  int constexpr nucleus_A(Code const code) {
    if (code == Code::Nucleus) {
      throw std::runtime_error("nucleus_A(Code::Nucleus) is impossible!");
    }
    return particle::detail::nucleusA[static_cast<CodeIntType>(code)];
  }

  int constexpr nucleus_Z(Code const code) {
    if (code == Code::Nucleus) {
      throw std::runtime_error("nucleus_Z(Code::Nucleus) is impossible!");
    }
    return particle::detail::nucleusZ[static_cast<CodeIntType>(code)];
  }

  bool constexpr is_nucleus(Code const code) {
    return (code == Code::Nucleus) || (nucleus_A(code) != 0);
  }

  //! the output stream operator for human-readable particle codes
  inline std::ostream& operator<<(std::ostream& stream, corsika::Code const code) {
    return stream << name(code);
  }

  //! convert PDG code to CORSIKA 8 internal code
  inline Code convert_from_PDG(PDGCode p) {
    static_assert(particle::detail::conversionArray.size() % 2 == 1);
    // this will fail, for the strange case where the maxPDG is negative...
    int constexpr maxPDG{(particle::detail::conversionArray.size() - 1) >> 1};
    auto const k = static_cast<PDGCodeType>(p);
    if (std::abs(k) <= maxPDG) {
      return particle::detail::conversionArray[k + maxPDG];
    } else {
      return particle::detail::conversionMap.at(p);
    }
  }

  //! returns mass of (A,Z) nucleus, disregarding binding energy
  HEPMassType nucleus_mass(const int A, const int Z) {
    auto const absA = std::abs(A);
    auto const absZ = std::abs(Z);
    return Proton::mass() * absZ + (absA - absZ) * Neutron::mass();
  }

} // namespace corsika
