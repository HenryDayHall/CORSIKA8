/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/process/InteractionHistogram.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>

namespace corsika {

  /*!
   * Wrapper around an InteractionProcess that fills histograms of the number
   * of calls to DoInteraction() binned in projectile energy (both in
   * lab and center-of-mass frame) and species
   */
  template <class TCountedProcess>
  class InteractionCounter
      : public InteractionProcess<InteractionCounter<TCountedProcess>> {

  public:
    InteractionCounter(TCountedProcess& process);

    template <typename TSecondaryView>
    void doInteraction(TSecondaryView& view);

    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const& particle) const;

    InteractionHistogram const& getHistogram() const;

  private:
    TCountedProcess& process_;
    InteractionHistogram histogram_;
  };

} // namespace corsika

#include <corsika/detail/framework/process/InteractionCounter.inl>
