/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/RadioProcess.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/modules/radio/propagators/SignalPath.hpp>
#include <cmath>

namespace corsika {

  /**
   * A concrete implementation of the ZHS algorithm.
   */
  template <typename TRadioDetector, typename TPropagator>
  class ZHS final : public RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>,
                                        TPropagator> {

  public:
    // an identifier for which algorithm was used
    static constexpr auto algorithm = "ZHS";

    /**
     * Construct a new ZHS instance.
     *
     * This forwards the detector and other arguments to
     * the RadioProcess parent.
     *
     */
    template <typename... TArgs>
    ZHS(TRadioDetector& detector, TArgs&&... args)
        : RadioProcess<TRadioDetector, ZHS, TPropagator>(detector, args...){};

    /**
     * Simulate the radio emission from a particle across a track.
     *
     * This must be provided by the TRadioImpl.
     *
     * @param particle    The current particle.
     * @param track       The current track.
     *
     */
    template <typename Particle>
    ProcessReturn simulate(Step<Particle> const& step) const;

  private:
    using Base =
        RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator>;
    using Base::antennas_;

  }; // END: class ZHS

} // namespace corsika

#include <corsika/detail/modules/radio/ZHS.inl>