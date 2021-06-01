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
#include <cmath>

namespace corsika {

  /**
   * A concrete implementation of the ZHS algorithm.
   */
  template <typename TRadioDetector, typename TPropagator>
  class ZHS final : public RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>,
                                        TPropagator> {

    using Base =
        RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator>;
    using Base::antennas_;

  public:
    //    using PotentialVector = QuantityVector<PotentialVectorType::dimension_type>;
    using ElectricFieldVector = QuantityVector<ElectricFieldType::dimension_type>;

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
        : RadioProcess<TRadioDetector, ZHS, TPropagator>(detector, args...) {}

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

      CORSIKA_LOG_INFO("Z  H  S");
      // TODO: think if we reuse these variables for the case of not being in the
      // Fraunhoffer approx.
      auto const startTime_{particle.getTime()}; // time at start point of track.
      auto const endTime_{particle.getTime() + track.getDuration()};    // time at end point of track.

      auto const startPoint_{track.getPosition(0)};
      auto const endPoint_{track.getPosition(1)};
      LengthType trackLength_{(startPoint_ - endPoint_).getNorm()};

      // track velocity
      auto const trackVelocity_{(track.getVelocity(0) + track.getVelocity(1)) / 2};

      // beta is defined as velocity / speed of light
      auto const beta_{trackVelocity_ / constants::c};

      // get particle charge
      auto const charge_{get_charge(particle.getPID())};

      // get "mid" position of the track geometrically
      auto const midVector_{(startPoint_ - endPoint_) / 2};
      auto const midPoint_{
          Point(midVector_.getCoordinateSystem(), midVector_.getComponents().getX(),
                midVector_.getComponents().getY(), midVector_.getComponents().getZ())};
      // changed if deltaT1_ > deltaT2_, so not const
      auto constants{charge_ / (4 * M_PI) / (constants::epsilonZero) / constants::c};
      // we loop over each antenna in the collection
      for (auto& antenna : antennas_.getAntennas()) {
        // Probably will be reused in the case of not being in Fraunhoffer.
        auto midPaths{this->propagator_.propagate(midPoint_, antenna.getLocation(), 1_m)};
        // Loop over midPaths to first check Fraunhoffer limit
        for (size_t i{0}; i < midPaths.size(); i++) {
          // Maybe I can recalculate fraunhLimit without Midpaths to avoid calculating
          // this third path which is something really slow, and only use path1 and path2.
          double const betaTimesK{beta_.dot(midPaths[i].emit_) / beta_.getNorm()};
          double const sinTheta2_{1. - betaTimesK * betaTimesK};
          // Parameter that determines the limit for the Fraunhoffer limit (probably
          // related to antenna sampling rate
          LengthType const lambda{constants::c/antenna.sample_rate_};
          double const fraunhLimit{sinTheta2_ * trackLength_ * trackLength_/ midPaths[i].R_distance_ / lambda};
          // Checks if we are in fraunhoffer domain (maybe it should be less?)
          if (fraunhLimit > 0.1) 
          {
            /// code for dividing track and calculating field.
            std::cout << "This code hasnt been implemented!" << std::endl;
          } else // Calculate vector potential of whole track
          {
            // paths from start of track to antenna
            auto paths1{
                this->propagator_.propagate(startPoint_, antenna.getLocation(), 1_m)};

            // paths from end of track to antenna
            auto paths2{
                this->propagator_.propagate(endPoint_, antenna.getLocation(), 1_m)};
            // First implementation, need to check if there are the same number of paths,
            // but once its working.

            // modified later if detectionTime1_ > detectionTime2_
            TimeType detectionTime1_{startTime_ + paths1[i].propagation_time_};
            TimeType detectionTime2_{endTime_ + paths2[i].propagation_time_};

            // make detectionTime1_ be the smallest time => changes step function order so
            // constants is changed to account for it
            if (detectionTime1_ > detectionTime2_) {
              detectionTime1_ = endTime_ + paths2[i].propagation_time_;
              detectionTime2_ = startTime_ + paths1[i].propagation_time_;
              constants = -constants;
            }

            double const startBin{std::floor(detectionTime1_ * antenna.sample_rate_)};
            double const endBin{std::floor(detectionTime2_ * antenna.sample_rate_)};

            auto const betaPerp_{midPaths[i].emit_.cross(beta_.cross(midPaths[i].emit_))};
            double const denominator{1 - midPaths[i].refractive_index_source_ *
                                             beta_.dot(midPaths[i].emit_)};

            // IMPORTANT!!!!! Using ElectricFieldVector instead of PotentialVector until
            // antenna is adapted
            if (startBin == endBin) {
              // track contained in bin
              // if not in Cerenkov angle then
              if (std::fabs(denominator) > 1.e-15L) {
                double const f{std::fabs((detectionTime2_ * antenna.sample_rate_ -
                                          detectionTime1_ * antenna.sample_rate_))};
                // should be PotentialVector const Vp_ =
                // betaPerp_.getComponents()/denominator/
                //                                    midPaths[i].R_distance_ * constants
                //                                    * f;
                // but to make it compile until antenna is adapted it stays like that to
                // test it.
                ElectricFieldVector const Vp_ = betaPerp_.getComponents() / denominator /
                                                midPaths[i].R_distance_ * constants * f /
                                                1_s;
                antenna.receive(detectionTime2_, betaPerp_, Vp_);
              } else { // If emission in Cerenkov angle => approximation
                double const f{(detectionTime2_ - detectionTime1_) *
                               antenna.sample_rate_};
                ElectricFieldVector const Vp_ = betaPerp_.getComponents() /
                                                midPaths[i].R_distance_ * constants * f /
                                                1_s;
                antenna.receive(detectionTime2_, betaPerp_, Vp_);
              } // end if Cerenkov angle approx
            } else {
              /*Track is contained in more than one bin*/
              int const numberOfBins{static_cast<int>(endBin - startBin)};
              // TODO: should we check for Cerenkov angle?
              // first contribution
              double f{std::fabs(startBin + 1. - detectionTime1_ * antenna.sample_rate_)};
              ElectricFieldVector Vp_ = betaPerp_.getComponents() * f * constants /
                                        denominator / midPaths[i].R_distance_ / 1_s;
              antenna.receive(detectionTime1_, betaPerp_, Vp_);
              // intermidiate contributions
              for (int it{1}; it < numberOfBins; ++it) {
                Vp_ = betaPerp_.getComponents() * constants / denominator /
                      midPaths[i].R_distance_ / 1_s;
                antenna.receive(
                    detectionTime1_ + static_cast<double>(it) / antenna.sample_rate_,
                    betaPerp_, Vp_);
              } // end loop over bins in which potential vector is not zero
              // final contribution
              f = std::fabs(detectionTime2_ * antenna.sample_rate_ - endBin);
              Vp_ = betaPerp_.getComponents() * f * constants / denominator /
                    midPaths[i].R_distance_ / 1_s;
              antenna.receive(detectionTime2_, betaPerp_, Vp_);
            } // end if statement for track in multiple bins

          } // finish if statement of track in fraunhoffer or not

        } // end loop over mid paths

      } // END: loop over antennas
      return ProcessReturn::Ok;
    } // end simulate

  }; // END: class ZHS

} // namespace corsika
