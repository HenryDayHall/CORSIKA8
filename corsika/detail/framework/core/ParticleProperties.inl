/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <cmath>
#include <corsika/framework/core/PhysicalUnits.hpp>

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

  inline std::ostream& operator<<(std::ostream& stream, corsika::Code const code) {
    return stream << name(code);
  }

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

  inline HEPMassType nucleus_mass(const int A, const int Z) {
    auto const absA = std::abs(A);
    auto const absZ = std::abs(Z);
    return mass(Code::Proton) * absZ + (absA - absZ) * mass(Code::Neutron);
  }

} // namespace corsika
