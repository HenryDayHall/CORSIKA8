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
    @ingroup Processes
    @{

   * Wrapper around an InteractionProcess that fills histograms of the number
   * of calls to `doInteraction()` binned in projectile energy (both in
   * lab and center-of-mass frame) and species
   *
   * Use by wrapping a normal InteractionProcess
   * @code{.cpp}
   * InteractionProcess collision1;
   * InteractionClounter<collision1> counted_collision1;
   * @endcode
   *
   */
  template <class TCountedProcess>
  class InteractionCounter
      : public InteractionProcess<InteractionCounter<TCountedProcess>> {

  public:
    InteractionCounter(TCountedProcess& process);

    //! wrapper around internall process doInteraction
    template <typename TSecondaryView>
    void doInteraction(TSecondaryView& view);

    ///! returns internal process getInteractionLength
    template <typename TParticle>
    GrammageType getInteractionLength(TParticle const& particle) const;

    /** returns the filles histograms
        @return InteractionHistogram, which contains the histogram data
    */
    InteractionHistogram const& getHistogram() const;

  private:
    TCountedProcess& process_;
    InteractionHistogram histogram_;
  };

  //! @}

} // namespace corsika

#include <corsika/detail/framework/process/InteractionCounter.inl>
