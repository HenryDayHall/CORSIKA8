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

namespace corsika {

  /**
   * A concrete implementation of the Enpoints formalism.
   */
  template <typename TRadioDetector, typename TPropagator>
  class CoREAS final : public RadioProcess<TRadioDetector, CoREAS<TRadioDetector, TPropagator>, TPropagator> {

    using Base = RadioProcess<TRadioDetector, CoREAS<TRadioDetector, TPropagator>, TPropagator>;
    using Base::detector_;

  public:
    using ElectricFieldVector =
    QuantityVector<ElectricFieldType::dimension_type>;
    /**
     * Construct a new CoREAS instance.
     *
     * This forwards the detector and other arguments to
     * the RadioProcess parent.
     *
     */
    template <typename... TArgs>
    CoREAS(TRadioDetector& detector, TArgs&&... args)
        : RadioProcess<TRadioDetector, CoREAS, TPropagator>(detector, args...){}

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
      auto startTime_ {particle.getTime()}; // time at start point of track.
      auto endTime_ {particle.getTime() - track.getDuration()}; // time at end point of track.

      // an alternative is the following which shouldn't work I think
//      auto startTime_ {particle.getTime()};
//      auto endTime_ {startTime_ + track.getDuration()};

      // beta is defined as velocity / speed of light
      auto startBeta_ {track.getVelocity(0) / constants::c};
      auto endBeta_ {track.getVelocity(1) / constants::c};

      // calculate gamma factor using beta (the proper way would be with energy over mass but not yet :( )
      auto startGamma_ {1. / sqrt(1. - (startBeta_ * startBeta_))};
      auto endGamma_ {1. / sqrt(1. - (endBeta_ * endBeta_))};

      // we loop over each antenna in the collection
      for (auto& antenna : detector_.getAntennas()) {

        // auto startPoint = /* TODO: get Point from Track */;
        auto startPoint = track.getPosition(0); // this MIGHT work and also get it out of the for loop?
        auto endPoint = track.getPosition(1); // I think we should get these guys out of the for loop
        // get the Path from the track to the antenna
        // This is a SignalPathCollection
        auto paths1{this->propagator_.propagate(startPoint, antenna.getLocation())};

        // now loop over the paths that we got above
        // Note: for the StraightPropagator, there will only be a single
        // path but other propagators may return more than one.
        for (auto const& path : paths1) {
//          auto startPoint_ {path.total_time_ + startTime_};
//          path.average_refractivity_;
//          path.emit_;
//          path.receive_;

          // combination along this path.
          // global time + time delay
          // and pass it to the antenna.
          // CoREAS calculation -> get ElectricFieldVector1
          antenna.receive(/* startPoint_, receive vector, ElectricFieldVector1 */);

        } // END: loop over paths

        // get the Path from the track to the antenna
        // This is a SignalPathCollection
        auto paths2{this->propagator_.propagate(endPoint, antenna.getLocation())};

        for (auto const& path : paths2) {
//          auto endPoint_ {path.total_time + endTime_};
//          path.average_refractivity_;
//          path.emit_;
//          path.receive_;

          // combination along this path.
          // global time + time delay
          // and pass it to the antenna.
          //CoREAS calculation -> get ElectricFieldVector2
          antenna.receive(/* endPoint_, receive vector, ElectricFieldVector2 */);

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
    }

  }; // END: class RadioProcess

} // namespace corsika
