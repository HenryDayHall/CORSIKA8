/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/interaction_counter/InteractionHistogram.hpp>

#include <fstream>
#include <string>

using namespace corsika::process::interaction_counter;

InteractionHistogram::InteractionHistogram()
    : inthist_cms_{detail::hist_factory(num_bins_cms, lower_edge_cms, upper_edge_cms)}
    , inthist_lab_{detail::hist_factory(num_bins_lab, lower_edge_lab, upper_edge_lab)} {}

void InteractionHistogram::fill(particles::Code projectile_id,
                                units::si::HEPEnergyType lab_energy,
                                units::si::HEPEnergyType mass_target, int A, int Z) {
  using namespace units::si;
  auto constexpr inv_eV = 1 / 1_eV;
  if (projectile_id == particles::Code::Nucleus) {
    auto const sqrtS =
        sqrt(A * A * (units::constants::nucleonMass * units::constants::nucleonMass) +
             mass_target * mass_target + 2 * lab_energy * mass_target);

    int32_t const pdg = 1'000'000'000l + Z * 10'000l + A * 10l;

    inthist_lab_(pdg, lab_energy * inv_eV);
    inthist_cms_(pdg, sqrtS * inv_eV);
  } else {
    auto const projectile_mass = particles::GetMass(projectile_id);
    auto const sqrtS = sqrt(projectile_mass * projectile_mass +
                            mass_target * mass_target + 2 * lab_energy * mass_target);

    inthist_cms_(static_cast<int>(particles::GetPDG(projectile_id)), sqrtS * inv_eV);
    inthist_lab_(static_cast<int>(particles::GetPDG(projectile_id)), lab_energy * inv_eV);
  }
}

void InteractionHistogram::saveLab(std::string const& filename,
                                   utl::SaveMode mode) const {
  corsika::utl::save_hist(inthist_lab_, filename, mode);
}

void InteractionHistogram::saveCMS(std::string const& filename,
                                   utl::SaveMode mode) const {
  corsika::utl::save_hist(inthist_cms_, filename, mode);
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
