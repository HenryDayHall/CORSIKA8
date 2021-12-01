/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/PhysicalGeometry.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>

/**
 * \file EnergyMomentumOperations.hpp
 *
 * Relativic energy momentum calculations.
 */

namespace corsika {

  auto constexpr get_total_energy_sqr(HEPMomentumType const p, HEPMassType const m) {
    return p * p + m * m;
  }

  HEPEnergyType constexpr get_total_energy(HEPMomentumType const p, HEPMassType const m) {
    return sqrt(get_total_energy_sqr(p, m));
  }

  HEPEnergyType constexpr get_kinetic_energy(HEPMomentumType const p,
                                             HEPMassType const m) {
    return get_total_energy(p, m) - m;
  }

} // namespace corsika