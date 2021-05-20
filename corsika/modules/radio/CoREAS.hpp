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
   * A concrete implementation of the Enpoints formalism. TODO: are there any limitations for the track length?
   */
  template <typename TRadioDetector, typename TPropagator>
  class CoREAS final : public RadioProcess<TRadioDetector, CoREAS<TRadioDetector, TPropagator>, TPropagator> {

    using Base = RadioProcess<TRadioDetector, CoREAS<TRadioDetector, TPropagator>, TPropagator>;
    using Base::antennas_;

  public:
    using ElectricFieldVector =
    QuantityVector<ElectricFieldType::dimension_type>;

    // an identifier for which algorithm was used
    static constexpr auto algorithm = "CoREAS";

    /**
     * Construct a new CoREAS instance.
     *
     * This forwards the detector and other arguments to
     * the RadioProcess parent.
     *
     */
    template <typename... TArgs>
    CoREAS(TRadioDetector& detector, TArgs&&... args)
        : RadioProcess<TRadioDetector, CoREAS, TPropagator>(detector, args...) {}

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
    ProcessReturn simulate(Particle& particle, Track const& track) {

      // get the global simulation time for that track. (best guess for now)
      auto startTime_{particle.getTime()}; // time at the start point of the track hopefully. I should use something similar to fCoreHitTime (?)
      auto endTime_{particle.getTime() + track.getDuration()};    // time at end point of track.

      if (startTime_ - endTime_ == 0_s) {
        return ProcessReturn::Ok;
      } else {

      // TODO: this should be fixed with the continuous processes new design, so we can get the energy at start and end of track for corrections
      // gamma factor is calculated using beta
      // auto startGamma_ {1. / sqrt(1. - (startBeta_ * startBeta_))};
      // auto endGamma_ {1. / sqrt(1. - (endBeta_ * endBeta_))};

      // get start and end position of the track
      auto startPoint_{track.getPosition(0)};
      auto endPoint_{track.getPosition(1)};
      // calculate the track length
      auto tracklength_ {(endPoint_ - startPoint_).getNorm()};

      // beta is velocity / speed of light. Start & end should be the same in endpoints!
      auto beta_ {(endPoint_ - startPoint_) / (constants::c * (endTime_ - startTime_))};

        // get particle charge
        auto const charge_{get_charge(particle.getPID())};

        // constants for electric field vector calculation
        auto constants_{charge_ / (4 * M_PI) / (constants::epsilonZero) / constants::c};

      // set threshold for application of ZHS-like approximation.
      const double approxThreshold_{1.0e-3};

      // loop over each antenna in the antenna collection (detector)
      for (auto& antenna : antennas_.getAntennas()) {

        // get the SignalPathCollection (path1) from the start "endpoint" to the antenna.
        auto paths1{this->propagator_.propagate(startPoint_, antenna.getLocation(), 1_m)}; // TODO: Add the stepsize to .propagate() at some point

        // get the SignalPathCollection (path2) from the end "endpoint" to the antenna.
        auto paths2{this->propagator_.propagate(endPoint_, antenna.getLocation(), 1_m)};

        // throw an exception if path sizes don't match
        try {
        // loop over both paths at once and directly compare 'start' and 'end' attributes
        for (size_t i = (paths1.size() == paths2.size()) ? 0 : throw (i = paths1.size());
             (i < paths1.size() && i < paths2.size()); i++) {

          // calculate preDoppler factor
          double preDoppler_{1. - paths1[i].refractive_index_source_ *
                       beta_.dot(paths1[i].emit_)};

          // check if preDoppler has become zero in case of refractive index of unity because of numerical limitations
          // here you might need std::fabs(preDoppler) in the if statement - same with post & mid
          if (preDoppler_ == 0) {
            // redo calculation with higher precision
            long double indexL_ {paths1[i].refractive_index_source_};
            long double betaX_ {static_cast<double>(beta_.getComponents().getX())};
            long double betaY_ {static_cast<double>(beta_.getComponents().getY())};
            long double betaZ_ {static_cast<double>(beta_.getComponents().getZ())};
            long double startX_ {static_cast<double>(paths1[i].emit_.getComponents().getX())};
            long double startY_ {static_cast<double>(paths1[i].emit_.getComponents().getY())};
            long double startZ_ {static_cast<double>(paths1[i].emit_.getComponents().getZ())};
            long double doppler = 1.0l - indexL_ * (betaX_ * startX_ +
                                                    betaY_ * startY_ + betaZ_ * startZ_);
            preDoppler_ = doppler;
          }

            // calculate postDoppler factor
          double postDoppler_{1. - paths2[i].refractive_index_source_ *
                       beta_.dot(paths2[i].emit_)};

          // check if postDoppler has become zero in case of refractive index of unity because of numerical limitations
          if (postDoppler_ == 0) {
            // redo calculation with higher precision
            long double indexL_ {paths2[i].refractive_index_source_};
            long double betaX_ {static_cast<double>(beta_.getComponents().getX())};
            long double betaY_ {static_cast<double>(beta_.getComponents().getY())};
            long double betaZ_ {static_cast<double>(beta_.getComponents().getZ())};
            long double endX_ {static_cast<double>(paths2[i].emit_.getComponents().getX())};
            long double endY_ {static_cast<double>(paths2[i].emit_.getComponents().getY())};
            long double endZ_ {static_cast<double>(paths2[i].emit_.getComponents().getZ())};
            long double doppler = 1.0l - indexL_ * (betaX_ * endX_ +
                                                    betaY_ * endY_ + betaZ_ * endZ_);
            postDoppler_ = doppler;
          }

              // calculate receive time for startpoint (aka time delay)
              auto startPointReceiveTime_{startTime_ + paths1[i].propagation_time_}; // TODO: time 0 is when the imaginary primary hits the ground

              // calculate receive time for endpoint
              auto endPointReceiveTime_{endTime_ + paths2[i].propagation_time_};

              // get unit vector for startpoint at antenna location
              auto ReceiveVectorStart_{paths1[i].receive_};

              // get unit vector for endpoint at antenna location
              auto ReceiveVectorEnd_{paths2[i].receive_};

              // perform ZHS-like calculation close to Cherenkov angle and for refractive index at antenna location greater than 1
              if ((paths1[i].refractive_index_destination_ > 1) &&
                  (std::fabs(preDoppler_) <= approxThreshold_ || std::fabs(postDoppler_) <= approxThreshold_)) {

                std::cout << "****************** ZHS-like approximation ******************" << "\n" << std::endl;

                // clear the existing paths for this particle and track, since we don't need them anymore
                paths1.clear();
                paths2.clear();

                // get global simulation time for the middle point of that track.
                TimeType midTime_{((startPoint_ - endPoint_).getNorm() / 2) /
                                  track.getVelocity(0).getNorm()};

                // get "mid" position of the track geometrically
                auto midVector_{(startPoint_ - endPoint_) / 2};
                auto midPoint_{Point(midVector_.getCoordinateSystem(),
                                     midVector_.getComponents().getX(),
                                     midVector_.getComponents().getY(),
                                     midVector_.getComponents().getZ())};

            // get the SignalPathCollection (path3) from the middle "endpoint" to the antenna.
            auto paths3{this->propagator_.propagate(midPoint_, antenna.getLocation(), 1_m)};

                // now loop over the paths for endpoint that we got above
                for (auto const& path : paths3) {

              auto const midPointReceiveTime_{midTime_ + path.propagation_time_};
              double midDoppler_{1. - path.refractive_index_source_ * beta_.dot(path.emit_)};

              // check if midDoppler has become zero because of numerical limitations
              if (midDoppler_ == 0) {
                // redo calculation with higher precision
                long double indexL_ {path.refractive_index_source_};
                long double betaX_ {static_cast<double>(beta_.getComponents().getX())};
                long double betaY_ {static_cast<double>(beta_.getComponents().getY())};
                long double betaZ_ {static_cast<double>(beta_.getComponents().getZ())};
                long double midX_ {static_cast<double>(path.emit_.getComponents().getX())};
                long double midY_ {static_cast<double>(path.emit_.getComponents().getY())};
                long double midZ_ {static_cast<double>(path.emit_.getComponents().getZ())};
                long double doppler = 1.0l - indexL_ * (betaX_ * midX_ +
                                                        betaY_ * midY_ + betaZ_ * midZ_);
                midDoppler_ = doppler;
              }

                  // change the values of the receive unit vectors of start and end
                  ReceiveVectorStart_ = path.receive_;
                  ReceiveVectorEnd_ = path.receive_;

              // CoREAS calculation -> get ElectricFieldVector for "midPoint"
              ElectricFieldVector EVmid_ = (path.emit_.cross(path.emit_.cross(beta_))).getComponents()
                                           / midDoppler_ / path.R_distance_ * constants_ * antenna.sample_rate_;

                  ElectricFieldVector EV1_{EVmid_};
                  ElectricFieldVector EV2_{-EVmid_};

              double deltaT_{(tracklength_ / (constants::c * tracklength_ / (constants::c * (endTime_ - startTime_))) *
                            std::fabs(midDoppler_)) / 1_s};     // TODO: Caution with this!

                  if (startPointReceiveTime_ < endPointReceiveTime_) // EVstart_ arrives earlier
                  {
                    startPointReceiveTime_ = midPointReceiveTime_ - 0.5 * deltaT_ * 1_s;
                    endPointReceiveTime_ = midPointReceiveTime_ + 0.5 * deltaT_ * 1_s;
                  } else // EVend_ arrives earlier
                  {
                    startPointReceiveTime_ = midPointReceiveTime_ + 0.5 * deltaT_ * 1_s;
                    endPointReceiveTime_ = midPointReceiveTime_ - 0.5 * deltaT_ * 1_s;
                  }

                  const long double gridResolution_{1 / antenna.sample_rate_ / 1_s};
                  deltaT_ = (endPointReceiveTime_ - startPointReceiveTime_) / 1_s;

                  // redistribute contributions over time scale defined by the observation time resolution
                  if (std::fabs(deltaT_) < gridResolution_) {

                    EV1_ *= std::fabs((deltaT_) / gridResolution_);
                    EV2_ *= std::fabs((deltaT_) / gridResolution_);

                    const long startBin = static_cast<long>(std::floor(
                        (startPointReceiveTime_ / 1_s) / gridResolution_ + 0.5l));
                    const long endBin = static_cast<long>(std::floor(
                        (endPointReceiveTime_ / 1_s) / gridResolution_ + 0.5l));
                    const double startBinFraction =
                        ((startPointReceiveTime_ / 1_s) / gridResolution_) -
                        std::floor((startPointReceiveTime_ / 1_s) / gridResolution_);
                    const double endBinFraction =
                        ((endPointReceiveTime_ / 1_s) / gridResolution_) -
                        std::floor((endPointReceiveTime_ / 1_s) / gridResolution_);

                    // only do timing modification if contributions would land in same bin
                    if (startBin == endBin) {

                      // if startE arrives before endE
                      if ((deltaT_) >= 0) {
                        if ((startBinFraction >= 0.5) &&
                            (endBinFraction >= 0.5)) // both points left of bin center
                        {
                          startPointReceiveTime_ -=
                              gridResolution_ * 1_s; // shift EV1_ to previous gridpoint
                        } else if ((startBinFraction < 0.5) &&
                                   (endBinFraction <
                                    0.5)) // both points right of bin center
                        {
                          endPointReceiveTime_ +=
                              gridResolution_ * 1_s; // shift EV2_ to next gridpoint
                        } else                       // points on both sides of bin center
                        {
                          const double leftDist = 1.0 - startBinFraction;
                          const double rightDist = endBinFraction;
                          // check if asymmetry to right or left
                          if (rightDist >= leftDist) {
                            endPointReceiveTime_ +=
                                gridResolution_ * 1_s; // shift EV2_ to next gridpoint
                          } else {
                            startPointReceiveTime_ -=
                                gridResolution_ * 1_s; // shift EV1_ to previous gridpoint
                          }
                        }
                      } else // if endE arrives before startE
                      {
                        if ((startBinFraction >= 0.5) &&
                            (endBinFraction >= 0.5)) // both points left of bin center
                        {
                          endPointReceiveTime_ -=
                              gridResolution_ * 1_s; // shift EV2_ to previous gridpoint
                        } else if ((startBinFraction < 0.5) &&
                                   (endBinFraction <
                                    0.5)) // both points right of bin center
                        {
                          startPointReceiveTime_ +=
                              gridResolution_ * 1_s; // shift EV1_ to next gridpoint
                        } else                       // points on both sides of bin center
                        {
                          const double leftDist = 1.0 - endBinFraction;
                          const double rightDist = startBinFraction;
                          // check if asymmetry to right or left
                          if (rightDist >= leftDist) {
                            startPointReceiveTime_ +=
                                gridResolution_ * 1_s; // shift EV1_ to next gridpoint
                          } else {
                            endPointReceiveTime_ -=
                                gridResolution_ * 1_s; // shift EV2_ to previous gridpoint
                          }
                        }
                      } // End of else statement
                    }   // End of if for startbin == endbin
                  }     // End of if deltaT < gridresolution

                  // TODO: Be very careful with this. Maybe the EVs should be fed after the for loop of paths3
                  antenna.receive(startPointReceiveTime_, ReceiveVectorStart_, EV1_);
                  antenna.receive(endPointReceiveTime_, ReceiveVectorEnd_, EV2_);
                  std::cout << "*****---------- signals received! ----------*****" << "\n" << std::endl;

                } // End of looping over paths3

              } // end of ZHS-like approximation
              else {

                std::cout << "****************** CoREAS ******************" << std::endl;

                // calculate electric field vector for startpoint
                ElectricFieldVector EV1_ = (paths1[i].emit_.cross(paths1[i].emit_.cross(beta_))).getComponents() /
                    preDoppler_ / paths1[i].R_distance_ * constants_ * antenna.sample_rate_;

                // calculate electric field vector for endpoint
                ElectricFieldVector EV2_ =
                    (paths2[i].emit_.cross(paths2[i].emit_.cross(beta_)))
                        .getComponents() /
                    postDoppler_ / paths2[i].R_distance_ * constants_ * (-1.0) *
                    antenna.sample_rate_;

                if ((preDoppler_ < 1.e-9) || (postDoppler_ < 1.e-9)) {

                  std::cout << "----- Doppler factors are less than 1.e-9 -----" << std::endl;

                  const long gridResolution_{1 / antenna.sample_rate_ / 1_s};
                  double deltaT_{(endPointReceiveTime_ - startPointReceiveTime_) / 1_s};

                  if (std::fabs(deltaT_) < gridResolution_) {

                    EV1_ *= std::fabs(deltaT_ / gridResolution_);
                    EV2_ *= std::fabs(deltaT_ / gridResolution_);

                    const long startBin = static_cast<long>(std::floor(
                        (startPointReceiveTime_ / 1_s) / gridResolution_ + 0.5l));
                    const long endBin = static_cast<long>(std::floor(
                        (endPointReceiveTime_ / 1_s) / gridResolution_ + 0.5l));
                    const double startBinFraction =
                        ((startPointReceiveTime_ / 1_s) / gridResolution_) -
                        std::floor((startPointReceiveTime_ / 1_s) / gridResolution_);
                    const double endBinFraction =
                        ((endPointReceiveTime_ / 1_s) / gridResolution_) -
                        std::floor((endPointReceiveTime_ / 1_s) / gridResolution_);

                    // only do timing modification if contributions would land in same bin
                    if (startBin == endBin) {

                      // if startE arrives before endE
                      if (deltaT_ >= 0) {
                        if ((startBinFraction >= 0.5) &&
                            (endBinFraction >= 0.5)) // both points left of bin center
                        {
                          startPointReceiveTime_ -=
                              gridResolution_ * 1_s; // shift EV1_ to previous gridpoint
                        } else if ((startBinFraction < 0.5) &&
                                   (endBinFraction <
                                    0.5)) // both points right of bin center
                        {
                          endPointReceiveTime_ +=
                              gridResolution_ * 1_s; // shift EV2_ to next gridpoint
                        } else                       // points on both sides of bin center
                        {
                          const double leftDist = 1.0 - startBinFraction;
                          const double rightDist = endBinFraction;
                          // check if asymmetry to right or left
                          if (rightDist >= leftDist) {
                            endPointReceiveTime_ +=
                                gridResolution_ * 1_s; // shift EV2_ to next gridpoint
                          } else {
                            startPointReceiveTime_ -=
                                gridResolution_ * 1_s; // shift EV1_ to previous gridpoint
                          }
                        }
                      } // End of if deltaT_ >=0
                    }   // End of if for startbin == endbin
                  }     // End of if deltaT < gridresolution
                }       // End of if that checks small doppler factors

                antenna.receive(startPointReceiveTime_, ReceiveVectorStart_, EV1_);
                antenna.receive(endPointReceiveTime_, ReceiveVectorEnd_, EV2_);
                std::cout << "*****---------- signals received! ----------*****" << "\n" << std::endl;

              } // End of else that does not perform ZHS-like approximation

            } // End of loop over both paths to get signal info
          }   // End of try block
          catch (size_t i) {
            std::cerr << " --- Signal Paths do not have the same size!!! --- "
                      << std::endl;
          }
        } // End of looping over antennas

        return ProcessReturn::Ok;
      }
    } // End of simulate method

  }; // END: class CoREAS

} // namespace corsika
