/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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
#include <bits/stdc++.h>

namespace corsika {

  /**
   * A concrete implementation of the ZHS algorithm.
   */
  template <typename TRadioDetector, typename TPropagator>
  class ZHS final : public RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator> {

      using Base = RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator>;
      using Base::detector_;
      
  public:
    using ElectricFieldVector =
    QuantityVector<ElectricFieldType::dimension_type>;
    /**
     * Construct a new ZHS instance.
     *
     * This forwards the detector and other arguments to
     * the RadioProcess parent.
     *
     */
    template <typename... TArgs>
    ZHS(TRadioDetector& detector, TArgs&&... args)
        : RadioProcess<TRadioDetector, ZHS, TPropagator>(detector, args...){}

    /**
     * Simulate the radio emission from a particle across a track.
     *
     * This must be provided by the TRadioImpl.
     *
     * @param particle    The current particle.
     * @param track       The current track.
     *
     */
    template <typename Particle, typename Track>
    ProcessReturn simulate(Particle& particle, Track const& track) const {

      //get global simulation time for that track. (This is my best guess for now)
      auto startTime_ {particle.getTime()
                       - track.getDuration()}; // time at start point of track.
      auto endTime_ {particle.getTime()}; // time at end point of track.
      auto midTime_ {(startTime_ + endTime_) / 2};

      auto startPoint_ = track.getPosition(0);

      // track velocity
      auto trackVelocity_ {(track.getVelocity(0) + track.getVelocity(1)) / 2};

      // beta is defined as velocity / speed of light
      auto beta_ { trackVelocity_ / constants::c};

      // get particle charge
      auto const charge_ {get_charge(particle.getPID())};

      // we loop over each antenna in the collection
      for (auto& antenna : detector_.getAntennas()) {

        // get the Path from the track to the antenna
        // This is a SignalPathCollection
        auto paths{this->propagator_.propagate(startPoint_, antenna.getLocation(), 1_nm)};

        // now loop over the paths that we got above
        // Note: for the StraightPropagator, there will only be a single
        // path but other propagators may return more than one.
        for (auto const& path : paths) {

          auto t1 = path.total_time_;

          QuantityVector<ElectricFieldType::dimension_type> v11{10_V / 1_m, 10_V / 1_m, 10_V / 1_m};
          // calculate the ZHS formalism for this particle-antenna
//          ElectricFieldVector EV_ = ((- constants) * trackVelocity_.dot(path.emit)) *
//          ((midTime_ + path.total_time_ - (1 - path.average_refractivity_ * beta_ *
//                                                                       acos(track.getDirection(0).dot(path.emit_))) * startTime_)
//           - (midTime_ + path.total_time_ - (1 - path.average_refractivity_ * beta_ *
//                                                                         acos(track.getDirection(0).dot(path.emit_))) * endTime_))
//           / (1 - path.average_refractivity_ * beta_ * acos(track.getDirection(0).dot(path.emit_)));
//
//          // pass it to the antenna
//          antenna.receive(midTime_ + path.total_time_, path.receive_, EV_);

        } // END: loop over paths

      } // END: loop over antennas
    }

    /**
     * Return the maximum step length for this particle and track.
     *
     * This must be provided by the TRadioImpl.
     *
     * @param particle    The current particle.
     * @param track       The current track.
     *
     * @returns The maximum length of this track.
     */
    template <typename Particle, typename Track>
    LengthType MaxStepLength(Particle const& particle,
                                        Track const& track) const {

      // TODO : This is where we control the maximum step size
      // of a particle track in order to maintain the accuracy
      // of the particular formalism.
      //
      // This is part of the ZHS / CoReas formalisms and can
      // be related from the magnetic field / acceleration, charge,
      // etc. of the particle.
      return 1000000000_m;
    }

  }; // END: class RadioProcess

} // namespace corsika
