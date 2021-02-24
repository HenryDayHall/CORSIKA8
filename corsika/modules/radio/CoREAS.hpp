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
#include <cmath>

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

      // set threshold for application of ZHS-like approximation
      const double approxThreshold_ {1.0e-3};

      //get global simulation time for that track. (This is my best guess for now)
      auto startTime_ {particle.getTime()
                      - track.getDuration()}; // time at start point of track.
      auto endTime_ {particle.getTime()}; // time at end point of track.

      // beta is defined as velocity / speed of light
      auto startBeta_ {track.getVelocity(0) / constants::c};
      auto endBeta_ {track.getVelocity(1) / constants::c};
      //TODO: check if they are different!!! They shouldn't!!!
//      auto startBeta_ {(track.getPosition(0) / (particle.getTime()
//                                                - track.getDuration()) ) / constants::c};
//      auto endBeta_ {(track.getPosition(1) / particle.getTime() ) / constants::c};

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

        ElectricFieldVector EV1_ {0_V / 0_m};
        ElectricFieldVector EV2_ {0_V / 0_m};
        ElectricFieldVector EV3_ {0_V / 0_m};

        auto startPointReceiveTime_ {0_ns};
        auto endPointReceiveTime_ {0_ns};

        // get the Path (path1) from the start "endpoint" to the antenna.
        // This is a SignalPathCollection
        auto paths1{this->propagator_.propagate(startPoint_, antenna.getLocation())};

        // set postDoppler to approximate threshold in advance and check after loop
        // if we need to perform ZHS-like approximation
        auto preDoppler_ {approxThreshold_};
        // now loop over the paths for startpoint that we got above
        for (auto const& path : paths1) {

          preDoppler_ = 1. - path.average_refractive_index_ *
                               startBeta_ * path.emit_;

          if (preDoppler_ > approxThreshold_) {
            startPointReceiveTime_ = path.total_time_ +
                                              startTime_ ; // might do it on the fly

            // CoREAS calculation -> get ElectricFieldVector1
            EV1_= (charge_ / constants::c) *
                path.receive_.cross(path.receive_.cross(startBeta_)) /
                (path.R_distance_ * preDoppler_);

            // pass it to the antenna
            antenna.receive(startPointReceiveTime_, path.receive_, EV1_);
          } else {
            continue;
          } // END preDoppler check

        } // END: loop over paths for startpoint

        // get the Path (path2) from the end "endpoint" to the antenna.
        // This is a SignalPathCollection
        auto paths2{this->propagator_.propagate(endPoint_, antenna.getLocation())};

        // set postDoppler to approximate threshold in advance and check after loop
        // if we need to perform ZHS-like approximation
        auto postDoppler_ {approxThreshold_};
        // now loop over the paths for endpoint that we got above
        for (auto const& path : paths2) {
          postDoppler_ = 1. - path.average_refractive_index_ *
                                endBeta_ * path.emit_;

          if (preDoppler_ > approxThreshold_) {
            auto endPointReceiveTime_ = path.total_time + endTime_ ; // might do it on the fly

            //CoREAS calculation -> get ElectricFieldVector2
          EV2_ = (charge_ / constants::c) *
                   path.receive_.cross(path.receive_.cross(endBeta_)) /
                   (path.R_distance_ * postDoppler_);

          // pass it to the antenna
          antenna.receive(endPointReceiveTime_, path.receive_, EV2_);
          } else {
            continue;
          } // END postDoppler check

        } // END: loop over paths for endpoint

        // perform ZHS-like calculation close to Cherenkov angle
        if (fabs(preDoppler_) <= approxThreshold_ || fabs(postDoppler_) <= approxThreshold_) {
            // get global simulation time for the middle point of that track. (This is my best guess for now)
            auto midTime_{particle.getTime() - (track.getDuration() / 2)};

            // beta is defined as velocity / speed of light
            auto midBeta_{track.getVelocity(0.5) / constants::c};
            //      auto midBeta_ {(track.getPosition(0.5) / (particle.getTime()
            //                                        - (track.getDuration() / 2) ) / constants::c};

            // calculate gamma factor using beta (the proper way would be with energy over mass)
            //          auto midGamma_ {1. / sqrt(1. - (midBeta_ * midBeta_))};

            // get start and end position of the track
            auto midPoint_{track.getPosition(0.5)};

            // get the Path (path3) from the middle "endpoint" to the antenna.
            // This is a SignalPathCollection
            auto paths3{this->propagator_.propagate(midPoint_, antenna.getLocation())};

            // now loop over the paths for endpoint that we got above
            for (auto const& path : paths3) {
              midDoppler_ = 1. - path.average_refractive_index_ * midBeta_ * path.emit_;

              auto const midPointReceiveTime_{path.total_time_ +
                                              midTime_}; // might do it on the fly

              // CoREAS calculation -> get ElectricFieldVector3 for "midPoint"
              EV3_ = (charge_ / constants::c) *
                     path.receive_.cross(path.receive_.cross(midBeta_)) /
                     (path.R_distance_ * midDoppler_);

              // pass it to the antenna but first check if "start" or "end" are double counted!!!
              if (fabs(preDoppler_) > approxThreshold_ &&
                  fabs(postDoppler_) <= approxThreshold_) {

                antenna.receive(startPointReceiveTime_, path.receive_, -EV1_);
                antenna.receive(midPointReceiveTime_, path.receive_, EV3_);
              } else if (fabs(preDoppler_) <= approxThreshold_ &&
                         fabs(postDoppler_) > approxThreshold_) {
                antenna.receive(endPointReceiveTime_, path.receive_, -EV2_);
                antenna.receive(midPointReceiveTime_, path.receive_, EV3_);

              } else {
                antenna.receive(midPointReceiveTime_, path.receive_, EV3_);
              } // end deleting double-counted values
            }
          } // end of ZHS-like approximation

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
