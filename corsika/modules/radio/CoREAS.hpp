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
      auto startTime_ {particle.getTime()
                      - track.getDuration()}; // time at start point of track.
      auto endTime_ {particle.getTime()}; // time at end point of track.

      // beta is defined as velocity / speed of light
      auto startBeta_ {track.getVelocity(0) / constants::c};
      auto endBeta_ {track.getVelocity(1) / constants::c};

      // calculate gamma factor using beta (the proper way would be with energy over mass)
      auto startGamma_ {1. / sqrt(1. - (startBeta_ * startBeta_))};
      auto endGamma_ {1. / sqrt(1. - (endBeta_ * endBeta_))};

      // get start and end position of the track
      auto startPoint_ {track.getPosition(0)};
      auto endPoint_ {track.getPosition(1)};

      // get particle charge
      auto const charge_ {get_charge(particle.getPID())};

      // we loop over each antenna in the collection
      for (auto& antenna : detector_.getAntennas()) {

        // get the Path (path1) from the start "endpoint" to the antenna.
        // This is a SignalPathCollection
        auto paths1{this->propagator_.propagate(startPoint_, antenna.getLocation())};
        auto R1_ {(startPoint_ - antenna.getLocation()).getNorm()};

        // now loop over the paths that we got above
        for (auto const& path : paths1) {
          auto startPointReceiveTime_ {path.total_time_ + startTime_}; // might do it on the fly

          // CoREAS calculation -> get ElectricFieldVector1
          ElectricFieldVector EV1_ {(charge_ / constants::c) *
                                   path.receive_.cross(path.receive_.cross(startBeta_)) /
                                        (R1_ * (1 - path.average_refractivity_ *
                                                       startBeta_ * path.receive_))};

          // pass it to the antenna
          antenna.receive(startPointReceiveTime_, path.receive_, EV1_);

        } // END: loop over paths

        // get the Path (path2) from the end "endpoint" to the antenna.
        // This is a SignalPathCollection
        auto paths2{this->propagator_.propagate(endPoint_, antenna.getLocation())};
        auto R2_ {(endPoint_ - antenna.getLocation()).getNorm()};

        for (auto const& path : paths2) {
          auto endPointReceiveTime_ {path.total_time + endTime_}; // might do it on the fly

          //CoREAS calculation -> get ElectricFieldVector2
          ElectricFieldVector EV2_ {(charge_ / constants::c) *
                                    path.receive_.cross(path.receive_.cross(endBeta_)) /
                                    (R2_ * (1 - path.average_refractivity_ *
                                                   endBeta_ * path.receive_))};

          // pass it to the antenna
          antenna.receive(endPointReceiveTime_, path.receive_, EV2_);

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
      return 1000000000000;
    }

  }; // END: class RadioProcess

} // namespace corsika
