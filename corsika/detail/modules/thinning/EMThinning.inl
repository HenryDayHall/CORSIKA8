/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>

namespace corsika {

  EMThinning::EMThinning(HEPEnergyType threshold, double maxWeight)
      : threshold_{threshold}
      , maxWeight_{maxWeight} {}

  template <typename TStackView>
  void EMThinning::doSecondaries(TStackView& view) {
    if (view.getSize() != 2) return;

    auto projectile = view.getProjectile();
    if (!is_em(projectile.getPID())) return;

    double const parentWeight = projectile.getWeight();
    if (parentWeight >= maxWeight_) { return; }

    auto particle1 = view.begin();
    auto particle2 = ++(view.begin());

    // skip non-EM interactions
    if (!is_em(particle1.getPID()) || !is_em(particle2.getPID())) { return; }

    /* skip high-energy interactions
     * We consider only the primary energy. A more sophisticated algorithm might
     * sort out above-threshold secondaries and apply thinning only on the subset of
     * those below the threshold.
     */
    if (projectile.getEnergy() > threshold_) { return; }

    corsika::HEPEnergyType const Esum = particle1.getEnergy() + particle2.getEnergy();

    double const p1 = particle1.getEnergy() / Esum;
    double const p2 = particle2.getEnergy() / Esum;

    // weight factors
    double const w1 = parentWeight / p1;
    double const w2 = parentWeight / p2;

    // max. allowed weight factor
    double const maxWeightFactor = maxWeight_ / parentWeight;

    if (w1 <= maxWeightFactor && w2 <= maxWeightFactor) { // apply Hillas thinning
      if (uniform_(rng_) <= p1) {                         // keep 1st with probability p1
        particle2.setWeight(0);
        particle1.setWeight(w1 * parentWeight);
      } else { // keep 2nd
        particle1.setWeight(0);
        particle2.setWeight(w2 * parentWeight);
      }
    } else { // weight limitation kicks in, do statistical thinning
      double const w1prime = std::min(w1, maxWeightFactor);
      double const w2prime = std::min(w2, maxWeightFactor);

      if (uniform_(rng_) * w1prime <= parentWeight) { // keep 1st
        particle1.setWeight(w1prime * parentWeight);
      } else {
        particle1.setWeight(0);
      }
      if (uniform_(rng_) * w2prime <= parentWeight) { // keep 2nd
        particle2.setWeight(w2prime * parentWeight);
      } else {
        particle2.setWeight(0);
      }
    }

    // erase discared particles
    // TODO: skip this for multithinning
    for (auto& p : view) {
      if (auto const w = p.getWeight(); w == 0) { p.erase(); }
    }
  }

} // namespace corsika
