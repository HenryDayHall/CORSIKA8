/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/InteractionHistogram.hpp>

namespace corsika {

  template <class TCountedProcess>
  inline InteractionCounter<TCountedProcess>::InteractionCounter(TCountedProcess& process)
      : process_(process) {}

  template <class TCountedProcess>
  template <typename TSecondaryView>
  inline void InteractionCounter<TCountedProcess>::doInteraction(TSecondaryView& view) {
    auto const projectile = view.getProjectile();
    auto const massNumber = projectile.getNode()
                                ->getModelProperties()
                                .getNuclearComposition()
                                .getAverageMassNumber();
    auto const massTarget = massNumber * constants::nucleonMass;
    histogram_.fill(projectile.getPID(), projectile.getEnergy(), massTarget);
    process_.doInteraction(view);
  }

  template <class TCountedProcess>
  template <typename TParticle>
  inline GrammageType InteractionCounter<TCountedProcess>::getInteractionLength(
      TParticle const& particle) const {
    return process_.getInteractionLength(particle);
  }

  template <class TCountedProcess>
  inline InteractionHistogram const& InteractionCounter<TCountedProcess>::getHistogram()
      const {
    return histogram_;
  }

} // namespace corsika
