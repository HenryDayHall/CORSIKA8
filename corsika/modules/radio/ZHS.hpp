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
  class ZHS final : public RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator> {

    using Base = RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator>;
    using Base::detector_;

  public:
//    using PotentialVector = QuantityVector<PotentialVectorType::dimension_type>;
    using ElectricFieldVector = QuantityVector<ElectricFieldType::dimension_type>;
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

      // TODO: think if we reuse these variables for the case of not being in the Fraunhoffer approx.
      auto const startTime_ {particle.getTime()
                       - track.getDuration()}; // time at start point of track.
      auto const endTime_ {particle.getTime()}; // time at end point of track.

      auto const startPoint_{track.getPosition(0)};
      auto const endPoint_{track.getPosition(1)};
      LengthType trackLength_ {(startPoint_ - endPoint_).getNorm()};

      // track velocity
      auto const trackVelocity_ {(track.getVelocity(0) + track.getVelocity(1)) / 2};

      // beta is defined as velocity / speed of light
      auto const beta_ { trackVelocity_ / constants::c};

      // get particle charge
      auto const charge_ {get_charge(particle.getPID())};

      // get "mid" position of the track geometrically
      auto const midVector_ { (startPoint_ - endPoint_) / 2 };
      auto const midPoint_ {Point(midVector_.getCoordinateSystem(), midVector_.getComponents().getX(),
                            midVector_.getComponents().getY(), midVector_.getComponents().getZ())};
      //changed if deltaT1_ > deltaT2_, so not const
      auto constants {charge_/ (4 * M_PI)/ (constants::epsilonZero) / constants::c};
      // we loop over each antenna in the collection
      for (auto& antenna : detector_.getAntennas()) {
        // Probably will be reused in the case of not being in Fraunhoffer.
        auto midPaths{this->propagator_.propagate(midPoint_, antenna.getLocation(), 1_m)};
        //Loop over midPaths to first check Fraunhoffer limit
        for (size_t i{0}; i < midPaths.size(); i++){
            double const betaTimesK {beta_.dot(midPaths[i].emit_)/beta_.getNorm()};
            double const slitSize_ {1. - betaTimesK * betaTimesK * trackLength_ * trackLength_ /1_m /1_m};
            //Parameter that determines the limit for the Fraunhoffer limit (probably related to antenna sampling rate
            double const lambda {0.1};
            double const fraunhLimit {slitSize_/midPaths[i].R_distance_/lambda * 1_m};
            if (fraunhLimit > 1.) // Checks if we are in fraunhoffer domain (maybe it should be less?)
            {
                ///code for dividing track and calculating field.
                std::cout << "This code hasnt been implemented!" << std::endl;
            }
            else // Calculate vector potential of whole track
            {
                // paths from start of track to antenna
                auto paths1{this->propagator_.propagate(startPoint_, antenna.getLocation(), 1_m)};

                // paths from end of track to antenna
                auto paths2{this->propagator_.propagate(endPoint_, antenna.getLocation(), 1_m)};
                // First implementation, need to check if there are the same number of paths, but once its working.

                //modified later if deltaT1_ > deltaT2_
                auto deltaT1_ {startTime_ + paths1[i].propagation_time_};
                auto deltaT2_ {endTime_ + paths2[i].propagation_time_};

                //make deltaT1_ be the smallest time => changes step function order so
                // constants is changed to account for it
                if (deltaT1_ > deltaT2_)
                {
                    deltaT1_ = {endTime_ + paths2[i].propagation_time_};
                    deltaT2_ = {startTime_ + paths1[i].propagation_time_};
                    constants = - constants;
                }


                long const startBin {std::floor(deltaT1_ * antenna.sample_rate_)};
                long const endBin {std::floor(deltaT2_ * antenna.sample_rate_)};

                auto const betaPerp_ {midPaths[i].emit_.cross(beta_.cross(midPaths[i].emit_))};
                double const denominator {1 - midPaths[i].refractive_index_source_ *
                                              beta_.dot(midPaths[i].emit_)};

                long const numberOfBins{endBin - startBin};
                //IMPORTANT!!!!! Using ElectricFieldVector instead of PotentialVector until antenna is adapted
                for (long int it{0}; it <= numberOfBins; ++it)
                {
                    if (startBin == endBin)//track contained in bin
                    {
//                        std::cout << "Track in one bin !" << std::endl;
                        if (std::fabs(denominator)>1.e-15L)//if not in Cerenkov angle then
                        {
                            double const f {std::fabs((deltaT2_ - deltaT1_) * antenna.sample_rate_)};
                            //should be PotentialVector const Vp_ = betaPerp_.getComponents()/denominator/
                            //                                    midPaths[i].R_distance_ * constants * f;
                            // but to make it compile until antenna is adapted it stays like that to test it.
                            ElectricFieldVector const Vp_ = betaPerp_.getComponents()/denominator/
                                    midPaths[i].R_distance_ * constants * f / 1_s;
//                            std::cout << "Vector Potential NOT in Cerenkov Angle: " << Vp_ << std::endl;
//                            std::cout << "Time " << deltaT2_ << std::endl;
                            antenna.receive(deltaT2_, betaPerp_, Vp_);
                        }
                        else//If emission in Cerenkov angle => approximation
                        {
                            double const f {(endTime_ - startTime_) * antenna.sample_rate_};
                            ElectricFieldVector const Vp_ = betaPerp_.getComponents()/
                                    midPaths[i].R_distance_ * constants * f / 1_s;
//                            std::cout << "Vector Potential in Cerenkov Angle: " << Vp_ << std::endl;
//                            std::cout << "Time " << deltaT1_ << std::endl;
                            antenna.receive(deltaT2_, betaPerp_, Vp_);
                        }//end if Cerenkov angle approx
                    }
                    //TODO: should we check for Cerenkov angle?
                    else //Track is contained in more than one bin
                    {
//                        std::cout << "Track in multiple bins !" << std::endl;
                        if (it == 0)//start of track
                        {
                            double const f {std::fabs(startTime_ * antenna.sample_rate_ - startBin)};
                            ElectricFieldVector const Vp_ = betaPerp_.getComponents() * f *
                                    constants/denominator/midPaths[i].R_distance_ / 1_s;
                            antenna.receive(deltaT1_, betaPerp_, Vp_);
                        }
                        else if (it == numberOfBins)//end of track
                        {
                            double const f {std::fabs(endTime_ * antenna.sample_rate_ - endBin)};
                            ElectricFieldVector const Vp_ = betaPerp_.getComponents() * f *
                                    constants / denominator / midPaths[i].R_distance_ / 1_s;
                            antenna.receive(deltaT2_, betaPerp_, Vp_);
                        }
                        else
                        {
                            ElectricFieldVector const Vp_ = betaPerp_.getComponents() * constants /
                                    denominator/midPaths[i].R_distance_ / 1_s;
                            antenna.receive(deltaT1_ + it / antenna.sample_rate_, betaPerp_, Vp_);
                        }
                    }
                }//end loop over bins in which potential vector is not zero

            }//finish if statement of track in fraunhoffer or not

        }//end loop over mid paths

        } // END: loop over antennas
        return ProcessReturn::Ok;
      } //end simulate


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