/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/interaction_counter/InteractionHistogram.hpp>

#include <corsika/process/InteractionProcess.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/setup/SetupStack.h>

namespace corsika::process::interaction_counter {
  /*!
   * Wrapper around an InteractionProcess that fills histograms of the number
   * of calls to DoInteraction() binned in projectile energy (both in
   * lab and center-of-mass frame) and species
   */
  template <class TCountedProcess>
  class InteractionCounter
      : public InteractionProcess<InteractionCounter<TCountedProcess>> {

    TCountedProcess& process_;
    InteractionHistogram histogram_;

  public:
    InteractionCounter(TCountedProcess& process)
        : process_(process) {}

    template <typename TSecondaryView>
    auto DoInteraction(TSecondaryView& view) {
      auto const projectile = view.GetProjectile();
      auto const massNumber = projectile.GetNode()
                                  ->GetModelProperties()
                                  .GetNuclearComposition()
                                  .GetAverageMassNumber();
      auto const massTarget = massNumber * units::constants::nucleonMass;

      if (auto const projectile_id = projectile.GetPID();
          projectile_id == particles::Code::Nucleus) {
        auto const A = projectile.GetNuclearA();
        auto const Z = projectile.GetNuclearZ();
        histogram_.fill(projectile_id, projectile.GetEnergy(), massTarget, A, Z);
      } else {
        histogram_.fill(projectile_id, projectile.GetEnergy(), massTarget);
      }
      return process_.DoInteraction(view);
    }

    template <typename TParticle>
    auto GetInteractionLength(TParticle const& particle) const {
      return process_.GetInteractionLength(particle);
    }

    auto const& GetHistogram() const { return histogram_; }
  };

} // namespace corsika::process::interaction_counter
