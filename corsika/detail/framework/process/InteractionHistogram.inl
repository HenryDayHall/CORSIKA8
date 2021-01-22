/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <fstream>
#include <string>

#include <corsika/framework/process/InteractionHistogram.hpp>
#include <corsika/detail/framework/process/InteractionHistogram.hpp> // for detail namespace

namespace corsika {

  InteractionHistogram::InteractionHistogram()
      : inthist_cms_{detail::hist_factory(num_bins_cms, lower_edge_cms, upper_edge_cms)}
      , inthist_lab_{detail::hist_factory(num_bins_lab, lower_edge_lab, upper_edge_lab)} {
  }

  void InteractionHistogram::fill(Code projectile_id, HEPEnergyType lab_energy,
                                  HEPEnergyType mass_target, int A, int Z) {
    auto constexpr inv_eV = 1 / 1_eV;
    if (projectile_id == Code::Nucleus) {
      auto const sqrtS = sqrt(A * A * (constants::nucleonMass * constants::nucleonMass) +
                              mass_target * mass_target + 2 * lab_energy * mass_target);

      int32_t const pdg = 1'000'000'000l + Z * 10'000l + A * 10l;

      inthist_lab_(pdg, lab_energy * inv_eV);
      inthist_cms_(pdg, sqrtS * inv_eV);
    } else {
      auto const projectile_mass = get_mass(projectile_id);
      auto const sqrtS = sqrt(projectile_mass * projectile_mass +
                              mass_target * mass_target + 2 * lab_energy * mass_target);

      inthist_cms_(static_cast<int>(get_PDG(projectile_id)), sqrtS * inv_eV);
      inthist_lab_(static_cast<int>(get_PDG(projectile_id)), lab_energy * inv_eV);
    }
  }

  InteractionHistogram& InteractionHistogram::operator+=(
      InteractionHistogram const& other) {
    inthist_lab_ += other.inthist_lab_;
    inthist_cms_ += other.inthist_cms_;

    return *this;
  }

  InteractionHistogram InteractionHistogram::operator+(InteractionHistogram other) const {
    other.inthist_lab_ += inthist_lab_;
    other.inthist_cms_ += inthist_cms_;

    return other;
  }
} // namespace corsika
