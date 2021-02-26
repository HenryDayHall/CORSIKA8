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

      // get the global simulation time for that track. (best guess for now)
      auto startTime_ {particle.getTime() - track.getDuration()}; // time at the start point of the track hopefully.
      auto endTime_ {particle.getTime()};

      // gamma factor is calculated using beta
//       auto startGamma_ {1. / sqrt(1. - (startBeta_ * startBeta_))};
//       auto endGamma_ {1. / sqrt(1. - (endBeta_ * endBeta_))};

      // get start and end position of the track
      auto startPoint_ {track.getPosition(0)};
      auto endPoint_ {track.getPosition(1)};

      // beta is velocity / speed of light. Start & end should be the same!
      auto beta_ {((endPoint_ - startPoint_) / (endTime_ - startTime_)).normalized()};

      // get particle charge
      auto const charge_ {get_charge(particle.getPID())};

      // set threshold for application of ZHS-like approximation.
      const double approxThreshold_ {1.0e-3};

      // we loop over each antenna in the collection.
      for (auto& antenna : detector_.getAntennas()) {

        std::vector<ElectricFieldVector> EVstart_;
        std::vector<ElectricFieldVector> EVend_;
        std::vector<TimeType> startTTimes_;
        std::vector<TimeType> endTTimes_;
        std::vector<double> preDoppler;
        std::vector<double> postDoppler;
        std::vector<QuantityVector<dimensionless_d>> ReceiveVectorsStart_;
        std::vector<QuantityVector<dimensionless_d>> ReceiveVectorsEnd_;

        // get the Path (path1) from the start "endpoint" to the antenna.
        // This is a Signal Path Collection
        auto paths1 {this->propagator_.propagate(startPoint_, antenna.getLocation(), 1_nm)}; // TODO: Need to add the stepsize to .propagate()!!!!

        // now loop over the paths for startpoint that we got above
        for (auto const& path : paths1) {

          // calculate preDoppler factor
          double preDoppler_{1. - path.average_refractive_index_ *
                                  beta_ * path.emit_};

          // store it to the preDoppler std::vector for later comparisons
          preDoppler.push_back(preDoppler_);

          // calculate receive times for startpoint
          auto startPointReceiveTime_ {path.total_time_ + startTime_};

          // store it to startTTimes_ std::vector for later use
          startTTimes_.push_back(startPointReceiveTime_);

          // store the receive unit vector
          ReceiveVectorsStart_.push_back(path.receive_);

          // calculate electric field vector for startpoint
          auto EV1_= (charge_ / constants::c) *
                     path.receive_.cross(path.receive_.cross(beta_)) /
                     (path.R_distance_ * preDoppler_);

          // store it to EVstart_ std::vector for later use
          EVstart_.push_back(EV1_);

        } // End of looping over paths1

        // get the Path (path2) from the end "endpoint" to the antenna.
        // This is a SignalPathCollection
        auto paths2 {this->propagator_.propagate(endPoint_, antenna.getLocation())};

        // now loop over the paths for endpoint that we got above
        for (auto const& path : paths2) {

          double postDoppler_{1. - path.average_refractive_index_ *
                                   beta_ * path.emit_}; // maybe this is path.receive_ (?)

          // store it to the postDoppler std::vector for later comparisons
          postDoppler.push_back(postDoppler_);

          // calculate receive times for endpoint
          auto endPointReceiveTime_ {path.total_time_ + endTime_};

          // store it to endTTimes_ std::vector for later use
          endTTimes_.push_back(endPointReceiveTime_);

          // store the receive unit vector
          ReceiveVectorsEnd_.push_back(path.receive_);

          // calculate electric field vector for endpoint
          auto EV2_= (charge_ / constants::c) *
                     path.receive_.cross(path.receive_.cross(beta_)) /
                     (path.R_distance_ * postDoppler_);

          // store it to EVstart_ std::vector for later use
          EVend_.push_back(EV2_);

        } // End of looping over paths2

        // start doing comparisons for preDoppler and postDoppler
        // first check that start and end paths have the same number of paths
        if (EVstart_.size() == EVend_.size()) {

          // use this to access different elements of std::vectors
          std::size_t index = 0;
          for (auto& preDoppler__ : preDoppler) {

            // redistribute contributions over time scale defined by the observation time resolution
            // this is to make sure that "start" and "end" won't end up in the same bin (xtensor)!!
            if ((preDoppler__ < 1.e-9) || (postDoppler.at(index) < 1.e-9)) {

              auto gridResolution_ {antenna.duration_};
              auto deltaT_ { endTTimes_.at(index) - startTTimes_.at(index) };

              if (fabs(deltaT_) < gridResolution_) {

                EVstart_.at(index) = EVstart_.at(index) * fabs(deltaT_ / gridResolution_);
                EVend_.at(index) = EVend_.at(index) * fabs(deltaT_ / gridResolution_);

                const long startBin = static_cast<long>(floor(startTTimes_.at(index)/gridResolution_+0.5l));
                const long endBin = static_cast<long>(floor(endTTimes_.at(index)/gridResolution_+0.5l));
                const double startBinFraction = (startTTimes_.at(index)/gridResolution_)-floor(startTTimes_.at(index)/gridResolution_);
                const double endBinFraction = (endTTimes_.at(index)/gridResolution_)-floor(endTTimes_.at(index)/gridResolution_);

                // only do timing modification if contributions would land in same bin
                if (startBin == endBin) {

                  // if startE arrives before endE
                  if (deltaT_ >= 0) {
                    if ((startBinFraction >= 0.5) && (endBinFraction >= 0.5)) // both points left of bin center
                    {
                      startTTimes_.at(index) = startTTimes_.at(index) - gridResolution_; // shift EV1_ to previous gridpoint
                    }
                    else if ((startBinFraction < 0.5) && (endBinFraction < 0.5)) // both points right of bin center
                    {
                      endTTimes_.at(index) = endTTimes_.at(index) + gridResolution_; // shift EV2_ to next gridpoint
                    }
                    else // points on both sides of bin center
                    {
                      const double leftDist = 1.0-startBinFraction;
                      const double rightDist = endBinFraction;
                      // check if asymmetry to right or left
                      if (rightDist >= leftDist)
                      {
                        endTTimes_.at(index) = endTTimes_.at(index) + gridResolution_; // shift EV2_ to next gridpoint
                      }
                      else
                      {
                        startTTimes_.at(index) = startTTimes_.at(index) - gridResolution_; // shift EV1_ to previous gridpoint
                      }
                    }
                  }
                  else // if endE arrives before startE
                  {
                    if ((startBinFraction >= 0.5) && (endBinFraction >= 0.5)) // both points left of bin center
                    {
                      endTTimes_.at(index) = endTTimes_.at(index) - gridResolution_; // shift EV2_ to previous gridpoint
                    }
                    else if ((startBinFraction < 0.5) && (endBinFraction < 0.5)) // both points right of bin center
                    {
                      startTTimes_.at(index) = startTTimes_.at(index) + gridResolution_; // shift EV1_ to next gridpoint
                    }
                    else // points on both sides of bin center
                    {
                      const double leftDist = 1.0-endBinFraction;
                      const double rightDist = startBinFraction;
                      // check if asymmetry to right or left
                      if (rightDist >= leftDist)
                      {
                        startTTimes_.at(index) = startTTimes_.at(index) + gridResolution_; // shift EV1_ to next gridpoint
                      }
                      else
                      {
                        endTTimes_.at(index) = endTTimes_.at(index) - gridResolution_; // shift EV2_ to previous gridpoint
                      }
                    }
                  } // End of else statement
                } // End of if for startbin == endbin
              } // End of if deltaT < gridresolution
            } // End of checking for very small doppler factors

            // perform ZHS-like calculation close to Cherenkov angle
            if (fabs(preDoppler__) <= approxThreshold_ || fabs(postDoppler.at(index)) <= approxThreshold_) {

              // get global simulation time for the middle point of that track. (This is my best guess for now)
              auto midTime_{particle.getTime() - (track.getDuration() / 2)};

              // get "mid" position of the track (that may not work properly)
              auto midPoint_{track.getPosition(0.5)};

              // get the Path (path3) from the middle "endpoint" to the antenna.
              // This is a SignalPathCollection
              auto paths3{this->propagator_.propagate(midPoint_, antenna.getLocation())};

//               std::size_t j_index {0}; // this will be useful for multiple paths (aka curved propagators)
              // now loop over the paths for endpoint that we got above
              for (auto const& path : paths3) {

//                 EVstart_.erase(EVstart_.begin() + index + j_index); // this should work for curved + curved propagators
//                 EVend_.erase(EVend_.begin() + index + j_index); // for now just use one index and not j_index since at the moment you are working with StraightPropagator

                auto const midPointReceiveTime_{path.total_time_ + midTime_};
                auto midDoppler_{1. - path.average_refractive_index_ * beta_ * path.emit_};

                // change the values of the receive unit vectors of start and end
                ReceiveVectorsStart_.at(index) = path.receive_;
                ReceiveVectorsEnd_.at(index) = path.receive_;

                // CoREAS calculation -> get ElectricFieldVector3 for "midPoint"
                ElectricFieldVector EVmid_ = (charge_ / constants::c) *
                                             path.receive_.cross(path.receive_.cross(beta_)) /
                                             (path.R_distance_ * midDoppler_);

//                 EVstart_.insert(EVstart_.begin() + index + j_index, EVmid_); // this should work for curved + curved propagators
//                 EVend_.insert(EVend_.begin() + index + j_index, - EVmid_); // for now just use one index and not j_index since at the moment you are working with StraightPropagator
                EVstart_.at(index) = EVmid_;
                EVend_.at(index) = - EVmid_;

                auto deltaT_{midPoint_.getNorm() / (constants::c * beta_ * fabs(midDoppler_))};

                if (startTTimes_.at(index) < endTTimes_.at(index)) // EVstart_ arrives earlier
                {
                  startTTimes_.at(index) = midPointReceiveTime_ - 0.5 * deltaT_;
                  endTTimes_.at(index) = midPointReceiveTime_ + 0.5 * deltaT_;
                }
                else // EVend_ arrives earlier
                {
                  startTTimes_.at(index) = midPointReceiveTime_ + 0.5 * deltaT_;
                  endTTimes_.at(index) = midPointReceiveTime_ - 0.5 * deltaT_;
                }

                const long double gridResolution_{antenna.duration_};
                deltaT_ = endTTimes_.at(index) - startTTimes_.at(index);

                // redistribute contributions over time scale defined by the observation time resolution
                if (fabs(deltaT_) < gridResolution_) {

                  EVstart_.at(index) = EVstart_.at(index) * fabs(deltaT_ / gridResolution_);
                  EVend_.at(index) = EVend_.at(index) * fabs(deltaT_ / gridResolution_);

                  const long startBin = static_cast<long>(floor(startTTimes_.at(index)/gridResolution_+0.5l));
                  const long endBin = static_cast<long>(floor(endTTimes_.at(index)/gridResolution_+0.5l));
                  const double startBinFraction = (startTTimes_.at(index)/gridResolution_)-floor(startTTimes_.at(index)/gridResolution_);
                  const double endBinFraction = (endTTimes_.at(index)/gridResolution_)-floor(endTTimes_.at(index)/gridResolution_);

                  // only do timing modification if contributions would land in same bin
                  if (startBin == endBin) {

                    // if startE arrives before endE
                    if (deltaT_ >= 0) {
                      if ((startBinFraction >= 0.5) && (endBinFraction >= 0.5)) // both points left of bin center
                      {
                        startTTimes_.at(index) = startTTimes_.at(index) - gridResolution_; // shift EV1_ to previous gridpoint
                      }
                      else if ((startBinFraction < 0.5) && (endBinFraction < 0.5)) // both points right of bin center
                      {
                        endTTimes_.at(index) = endTTimes_.at(index) + gridResolution_; // shift EV2_ to next gridpoint
                      }
                      else // points on both sides of bin center
                      {
                        const double leftDist = 1.0-startBinFraction;
                        const double rightDist = endBinFraction;
                        // check if asymmetry to right or left
                        if (rightDist >= leftDist)
                        {
                          endTTimes_.at(index) = endTTimes_.at(index) + gridResolution_; // shift EV2_ to next gridpoint
                        }
                        else
                        {
                          startTTimes_.at(index) = startTTimes_.at(index) - gridResolution_; // shift EV1_ to previous gridpoint
                        }
                      }
                    }
                    else // if endE arrives before startE
                    {
                      if ((startBinFraction >= 0.5) && (endBinFraction >= 0.5)) // both points left of bin center
                      {
                        endTTimes_.at(index) = endTTimes_.at(index) - gridResolution_; // shift EV2_ to previous gridpoint
                      }
                      else if ((startBinFraction < 0.5) && (endBinFraction < 0.5)) // both points right of bin center
                      {
                        startTTimes_.at(index) = startTTimes_.at(index) + gridResolution_; // shift EV1_ to next gridpoint
                      }
                      else // points on both sides of bin center
                      {
                        const double leftDist = 1.0-endBinFraction;
                        const double rightDist = startBinFraction;
                        // check if asymmetry to right or left
                        if (rightDist >= leftDist)
                        {
                          startTTimes_.at(index) = startTTimes_.at(index) + gridResolution_; // shift EV1_ to next gridpoint
                        }
                        else
                        {
                          endTTimes_.at(index) = endTTimes_.at(index) - gridResolution_; // shift EV2_ to previous gridpoint
                        }
                      }
                    } // End of else statement
                  } // End of if for startbin == endbin
                } // End of if deltaT < gridresolution

              } // End of looping over paths3

            } // end of ZHS-like approximation

            // Feed start and end to the antenna
            antenna.receive(startTTimes_.at(index), ReceiveVectorsStart_.at(index), EVstart_.at(index));
            antenna.receive(endTTimes_.at(index), ReceiveVectorsEnd_.at(index), EVend_.at(index));

            // update index
            index = index + 1;

          } // End of for loop for preDoppler factor (this includes checking for postDoppler factors)

        } // End of checking of vector sizes

      } // End of looping over the antennas.

    } // End of simulate method.


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
      return 1000000000000_m;
    }

  }; // END: class RadioProcess

} // namespace corsika
