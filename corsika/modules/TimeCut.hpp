/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika {

  /*
   * Simple TimeCut process. Stops the sequence at the indicated time
   */
  class TimeCut : public ContinuousProcess<TimeCut> {

  public:
    TimeCut(TimeType const time);

    template <typename Particle, typename Track>
    ProcessReturn doContinuous(
        Particle const& vParticle, Track const& vTrajectory,
        const bool limitFlag = false); // this is not used for TimeCut
    template <typename Particle, typename Track>
    LengthType getMaxStepLength(Particle const&, Track const&) {
      return meter * std::numeric_limits<double>::infinity();
    }

  private:
    TimeType time_;
  };

} // namespace corsika

#include <corsika/detail/modules/TimeCut.inl>
