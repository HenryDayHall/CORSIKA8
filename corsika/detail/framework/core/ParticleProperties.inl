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
#include <corsika/framework/core/Logging.hpp>

namespace corsika {

  inline HEPEnergyType constexpr calculate_kinetic_energy_threshold(Code const code) {
    if (is_nucleus(code)) return particle::detail::threshold_nuclei;
    return particle::detail::thresholds[static_cast<CodeIntType>(code)];
  }

  inline void constexpr set_kinetic_energy_threshold(Code const code,
                                                     HEPEnergyType const val) {
    if (is_nucleus(code))
      particle::detail::threshold_nuclei = val;
    else
      particle::detail::thresholds[static_cast<CodeIntType>(code)] = val;
  }

  inline HEPMassType constexpr get_mass(Code const code) {
    if (is_nucleus(code)) { return get_nucleus_mass(code); }
    return particle::detail::masses[static_cast<CodeIntType>(code)];
  }

  inline bool constexpr is_nucleus(Code const code) { return code >= Code::Nucleus; }

  inline Code constexpr get_nucleus_code(size_t const A,
                                         size_t const Z) { // 10LZZZAAAI
    if (Z > A) { throw std::runtime_error("Z cannot be larger than A in nucleus."); }
    return static_cast<Code>(static_cast<CodeIntType>(Code::Nucleus) + Z * 10000 +
                             A * 10);
  }

  inline size_t constexpr get_nucleus_Z(Code const code) {
    return (static_cast<CodeIntType>(code) % static_cast<CodeIntType>(Code::Nucleus)) /
           10000;
  }

  inline size_t constexpr get_nucleus_A(Code const code) {
    return (static_cast<CodeIntType>(code) % 10000) / 10;
  }

  inline PDGCode constexpr get_PDG(Code const code) {
    if (code < Code::Nucleus) {
      return particle::detail::pdg_codes[static_cast<CodeIntType>(code)];
    }
    size_t const Z = get_nucleus_Z(code);
    size_t const A = get_nucleus_A(code);
    return static_cast<PDGCode>(static_cast<CodeIntType>(Code::Nucleus) + Z * 10000 +
                                A * 10); // 10LZZZAAAI
  }

  inline int16_t constexpr get_charge_number(Code const code) {
    if (is_nucleus(code)) return get_nucleus_Z(code);
    return particle::detail::electric_charges[static_cast<CodeIntType>(code)];
  }

  inline ElectricChargeType constexpr get_charge(Code const code) {
    return get_charge_number(code) * constants::e;
  }

  inline std::string_view constexpr get_name(Code const code) {
    if (is_nucleus(code)) { return "nucleus"; }
    return particle::detail::names[static_cast<CodeIntType>(code)];
  }

  inline TimeType constexpr get_lifetime(Code const p) {
    return particle::detail::lifetime[static_cast<CodeIntType>(p)] * second;
  }

  inline bool constexpr is_hadron(Code const code) {
    if (is_nucleus(code)) return true;
    return particle::detail::isHadron[static_cast<CodeIntType>(code)];
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

  inline std::ostream& operator<<(std::ostream& stream, corsika::Code const code) {
    return stream << get_name(code);
  }

  inline Code convert_from_PDG(PDGCode const p) {
    static_assert(particle::detail::conversionArray.size() % 2 == 1);
    // this will fail, for the strange case where the maxPDG is negative...
    int constexpr maxPDG{(particle::detail::conversionArray.size() - 1) >> 1};
    auto const k = static_cast<PDGCodeIntType>(p);
    if (std::abs(k) <= maxPDG) {
      return particle::detail::conversionArray[k + maxPDG];
    } else {
      return particle::detail::conversionMap.at(p);
    }
  }

  inline HEPMassType constexpr get_nucleus_mass(Code const code) {
    unsigned int const A = get_nucleus_A(code);
    unsigned int const Z = get_nucleus_Z(code);
    return get_nucleus_mass(A, Z);
  }

  inline HEPMassType constexpr get_nucleus_mass(unsigned int const A,
                                                unsigned int const Z) {
    return get_mass(Code::Proton) * Z + (A - Z) * get_mass(Code::Neutron);
  }

  inline std::string_view get_nucleus_name(Code const code) {
    size_t const A = get_nucleus_A(code);
    size_t const Z = get_nucleus_Z(code);
    return fmt::format("Nucleus_A{}_Z{}", A, Z);
  }

  inline std::initializer_list<Code> constexpr get_all_particles() {
    return particle::detail::all_particles;
  }

} // namespace corsika
