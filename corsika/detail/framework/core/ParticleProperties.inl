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

  inline HEPEnergyType constexpr get_energy_threshold(Code const p) {
    return particle::detail::thresholds[static_cast<CodeIntType>(p)];
  }

  inline void constexpr set_energy_threshold(Code const p, HEPEnergyType const val) {
    particle::detail::thresholds[static_cast<CodeIntType>(p)] = val;
  }

  inline HEPMassType constexpr get_mass(Code const p) {
    if (p == Code::Nucleus)
      throw std::runtime_error("Cannot GetMass() of particle::Nucleus -> unspecified");
    return particle::detail::masses[static_cast<CodeIntType>(p)];
  }

  inline PDGCode constexpr get_PDG(Code const p) {
    return particle::detail::pdg_codes[static_cast<CodeIntType>(p)];
  }

  inline int16_t constexpr get_charge_number(Code const code) {
    if (code == Code::Nucleus)
      throw std::runtime_error("charge of particle::Nucleus undefined");
    return particle::detail::electric_charges[static_cast<CodeIntType>(code)];
  }

  inline ElectricChargeType constexpr get_charge(Code const code) {
    return get_charge_number(code) * constants::e;
  }

  inline std::string_view constexpr get_name(Code const code) {
    return particle::detail::names[static_cast<CodeIntType>(code)];
  }

  inline TimeType constexpr get_lifetime(Code const p) {
    return particle::detail::lifetime[static_cast<CodeIntType>(p)] * second;
  }

  inline bool constexpr is_hadron(Code const p) {
    return particle::detail::isHadron[static_cast<CodeIntType>(p)];
  }

  inline bool constexpr is_em(Code const c) {
    return c == Code::Electron || c == Code::Positron || c == Code::Photon;
  }

  inline bool constexpr is_muon(Code const c) {
    return c == Code::MuPlus || c == Code::MuMinus;
  }

  inline bool constexpr is_neutrino(Code const c) {
    return c == Code::NuE || c == Code::NuMu || c == Code::NuTau || c == Code::NuEBar ||
           c == Code::NuMuBar || c == Code::NuTauBar;
  }

  inline int constexpr get_nucleus_A(Code const code) {
    if (code == Code::Nucleus) {
      throw std::runtime_error("get_nucleus_A(Code::Nucleus) is impossible!");
    }
    return particle::detail::nucleusA[static_cast<CodeIntType>(code)];
  }

  inline int constexpr get_nucleus_Z(Code const code) {
    if (code == Code::Nucleus) {
      throw std::runtime_error("get_nucleus_Z(Code::Nucleus) is impossible!");
    }
    return particle::detail::nucleusZ[static_cast<CodeIntType>(code)];
  }

  inline bool constexpr is_nucleus(Code const code) {
    return (code == Code::Nucleus) || (get_nucleus_A(code) != 0);
  }

  inline std::ostream& operator<<(std::ostream& stream, corsika::Code const code) {
    return stream << get_name(code);
  }

  inline Code convert_from_PDG(PDGCode const p) {
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

  inline HEPMassType get_nucleus_mass(unsigned int const A, unsigned int const Z) {
    return get_mass(Code::Proton) * Z + (A - Z) * get_mass(Code::Neutron);
  }

  inline std::initializer_list<Code> constexpr get_all_particles() {
    return particle::detail::all_particles;
  }

} // namespace corsika
