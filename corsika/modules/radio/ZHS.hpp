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
        using VectorPotential = Vector<VectorPotentialType::dimension_type>;
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
        ProcessReturn simulate(Particle const& particle, Track const& track) const {

            auto const startTime{particle.getTime()};
            auto const endTime{particle.getTime() + track.getDuration()};

            if (startTime - endTime == 0_s) {
              return ProcessReturn::Ok;
            } else {

              auto const startPoint{track.getPosition(0)};
              auto const endPoint{track.getPosition(1)};
              LengthType trackLength{(startPoint - endPoint).getNorm()};

              auto const betaModule{(endPoint - startPoint).getNorm() /
                                    (constants::c * (endTime - startTime))};
              auto const beta{(endPoint - startPoint).normalized() * betaModule};

              auto const charge{get_charge(particle.getPID())};

              // // get "mid" position of the track geometrically

              auto halfVector{(startPoint - endPoint) / 2};
              auto midPoint{endPoint + halfVector};

              auto const constants{charge / (4 * M_PI) / (constants::epsilonZero) /
                              constants::c};

              // we loop over each antenna in the collection
              for (auto& antenna : antennas_.getAntennas()) {
                auto midPaths{
                    this->propagator_.propagate(midPoint, antenna.getLocation(), 1_m)};
                // Loop over midPaths, first check Fraunhoffer limit
                for (size_t i{0}; i < midPaths.size(); i++) {
                  double const uTimesK{beta.dot(midPaths[i].emit_) / betaModule};
                  double const sinTheta2{1. - uTimesK * uTimesK};
                  LengthType const lambda{constants::c / antenna.sample_rate_};
                  double const fraunhLimit{sinTheta2 * trackLength * trackLength /
                                           midPaths[i].R_distance_ / lambda * 2 * M_PI};
                  // Checks if we are in fraunhoffer domain
                  if (fraunhLimit > 1.0) {
                    /// code for dividing track and calculating field.
                    double const nSubTracks{sqrt(fraunhLimit) + 1};
                    auto const step{(endPoint - startPoint) / nSubTracks};
                    TimeType const timeStep{(endTime - startTime) / nSubTracks};
                    // energy should be divided up when it is possible to get the energy at end of track!!!!
                    auto point1{startPoint};
                    TimeType time1{startTime};
                    for (int j{0}; j < nSubTracks; j++) {
                      auto point2{point1 + step};
                      TimeType time2{time1 + timeStep};
                      auto newHalfVector{(point1 - point2) / 2.};
                      auto newMidPoint{point2 + newHalfVector};
                      auto newMidPaths{this->propagator_.propagate(
                          newMidPoint, antenna.getLocation(), 1_m)};
                      // A function for calculating the field should be made since it is repeated later
                      for (size_t k{0}; k < newMidPaths.size(); k++) {

                        double const n_source{newMidPaths[k].refractive_index_source_};

                        double const betaTimesK{beta.dot(newMidPaths[k].emit_)};
                        TimeType midTime{(time1 + time2) / 2.};
                        TimeType detectionTime1{time1 + newMidPaths[k].propagation_time_ -
                                                n_source * betaTimesK *
                                                    (time1 - midTime)};
                        TimeType detectionTime2{time2 + newMidPaths[k].propagation_time_ -
                                                n_source * betaTimesK *
                                                    (time2 - midTime)};

                        // make detectionTime1_ be the smallest time => 
                        // changes step function order so sign is changed to account for it
                        double sign = 1.;
                        if (detectionTime1 > detectionTime2) {
                          detectionTime1 = time2 + newMidPaths[k].propagation_time_ -
                                           n_source * betaTimesK * (time2 - midTime);
                          detectionTime2 = time1 + newMidPaths[k].propagation_time_ -
                                           n_source * betaTimesK * (time1 - midTime);
                          sign = -1.;
                        } // end if statement for time structure

                        double const startBin{
                            std::floor((detectionTime1 - antenna.start_time_) *
                                           antenna.sample_rate_ +
                                       0.5)};
                        double const endBin{
                            std::floor((detectionTime2 - antenna.start_time_) *
                                           antenna.sample_rate_ +
                                       0.5)};

                        auto const betaPerp{
                            newMidPaths[k].emit_.cross(beta.cross(newMidPaths[k].emit_))};
                        double const denominator{1. - n_source * betaTimesK};

                        if (startBin == endBin) {
                          // track contained in bin
                          // if not in Cerenkov angle then
                          if (std::fabs(denominator) > 1.e-15) {
                            double const f{
                                std::fabs((detectionTime2 * antenna.sample_rate_ -
                                           detectionTime1 * antenna.sample_rate_))};
                            VectorPotential const Vp = betaPerp * sign * constants * f /
                                                       denominator / 
                                                       newMidPaths[k].R_distance_;
                            antenna.receive(detectionTime2, betaPerp, Vp);
                          } else { // If emission in Cerenkov angle => approximation
                            double const f{time2*antenna.sample_rate_ -
                                                time1*antenna.sample_rate_};
                            VectorPotential const Vp = betaPerp * sign * constants * f /
                                                 newMidPaths[k].R_distance_;
                            antenna.receive(detectionTime2, betaPerp, Vp);
                          } // end if Cerenkov angle approx
                        } else {
                          /*Track is contained in more than one bin*/
                          int const numberOfBins{static_cast<int>(endBin - startBin)};
                          // first contribution/ plus 1 bin minus 0.5 from new antenna ruonding
                          double f{std::fabs(startBin + 0.5 -
                                             (detectionTime1 - antenna.start_time_) *
                                                 antenna.sample_rate_)};
                          VectorPotential Vp = betaPerp * sign * constants * f / denominator /
                                               newMidPaths[k].R_distance_;
                          antenna.receive(detectionTime1, betaPerp, Vp);
                          // intermidiate contributions
                          for (int it{1}; it < numberOfBins; ++it) {
                            Vp = betaPerp * constants / denominator /
                                 newMidPaths[k].R_distance_;
                            antenna.receive(detectionTime1 + static_cast<double>(it) /
                                                                 antenna.sample_rate_,
                                            betaPerp, Vp);
                          } // end loop over bins in which potential vector is not zero
                          // final contribution// f +0.5 from new antenna rounding
                          f = std::fabs((detectionTime2 - antenna.start_time_) *
                                            antenna.sample_rate_ +
                                        0.5 - endBin);
                          Vp = betaPerp * sign * constants * f / denominator /
                               newMidPaths[k].R_distance_;
                          antenna.receive(detectionTime2, betaPerp, Vp);
                        } // end if statement for track in multiple bins

                      } // end of loop over newMidPaths
                      // update points for next sub track
                      point1 = point1 + step;
                      time1 = time1 + timeStep;
                    }

                  } else // Calculate vector potential of whole track
                  {
                    double const n_source{midPaths[i].refractive_index_source_};

                    double const betaTimesK{beta.dot(midPaths[i].emit_)};
                    TimeType midTime{(startTime + endTime) / 2};
                    TimeType detectionTime1{startTime + midPaths[i].propagation_time_ -
                                            n_source * betaTimesK *
                                                (startTime - midTime)};
                    TimeType detectionTime2{endTime + midPaths[i].propagation_time_ -
                                            n_source * betaTimesK * (endTime - midTime)};

                    // make detectionTime1_ be the smallest time => 
                    // changes step function order so sign is changed to account for it
                    double sign = 1.;
                    if (detectionTime1 > detectionTime2) {
                      detectionTime1 = endTime + midPaths[i].propagation_time_ -
                                       n_source * betaTimesK * (endTime - midTime);
                      detectionTime2 = startTime + midPaths[i].propagation_time_ -
                                       n_source * betaTimesK * (startTime - midTime);
                      sign = -1.;
                    }// end if statement for time structure

                    double const startBin{std::floor(
                        (detectionTime1 - antenna.start_time_) * antenna.sample_rate_ +
                        0.5)};
                    double const endBin{std::floor(
                        (detectionTime2 - antenna.start_time_) * antenna.sample_rate_ +
                        0.5)};

                    auto const betaPerp{
                        midPaths[i].emit_.cross(beta.cross(midPaths[i].emit_))};
                    double const denominator{1. - midPaths[i].refractive_index_source_ *
                                                      betaTimesK};

                    if (startBin == endBin) {
                      // track contained in bin
                      // if not in Cerenkov angle then
                      if (std::fabs(denominator) > 1.e-15) {
                        double const f{
                            std::fabs((detectionTime2 * antenna.sample_rate_ -
                                       detectionTime1 * antenna.sample_rate_))};

                        VectorPotential const Vp = betaPerp * sign * constants * f /
                                                  denominator / midPaths[i].R_distance_;
                        antenna.receive(detectionTime2, betaPerp, Vp);
                      } else { // If emission in Cerenkov angle => approximation
                        double const f{endTime*antenna.sample_rate_ - 
                                        startTime*antenna.sample_rate_};
                        VectorPotential const Vp = betaPerp * sign * constants * f /
                                                   midPaths[i].R_distance_;
                        antenna.receive(detectionTime2, betaPerp, Vp);
                      } // end if Cerenkov angle approx
                    } else {
                      /*Track is contained in more than one bin*/
                      int const numberOfBins{static_cast<int>(endBin - startBin)};
                      // TODO: should we check for Cerenkov angle?
                      // first contribution
                      double f{std::fabs(startBin + 0.5 -
                                         (detectionTime1 - antenna.start_time_) *
                                             antenna.sample_rate_)};
                      VectorPotential Vp = betaPerp * sign * constants * f /
                                          denominator / midPaths[i].R_distance_;
                      antenna.receive(detectionTime1, betaPerp, Vp);
                      // intermidiate contributions
                      for (int it{1}; it < numberOfBins; ++it) {
                        Vp = betaPerp * sign * constants / denominator /
                             midPaths[i].R_distance_;
                        antenna.receive(detectionTime1 + static_cast<double>(it) /
                                                             antenna.sample_rate_,
                                        betaPerp, Vp);
                      } // end loop over bins in which potential vector is not zero
                      // final contribution
                      f = std::fabs((detectionTime2 - antenna.start_time_) *
                                        antenna.sample_rate_ +
                                    0.5 - endBin);
                      Vp = betaPerp * sign * constants * f / denominator /
                           midPaths[i].R_distance_;
                      antenna.receive(detectionTime2, betaPerp, Vp);
                    } // end if statement for track in multiple bins

                  } // finish if statement of track in fraunhoffer or not

                } // end loop over mid paths

              } // END: loop over antennas
              return ProcessReturn::Ok;
            }
        } // end simulate

    }; // END: class ZHS

} // namespace corsika
