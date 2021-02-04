/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/antennas/Antenna.hpp>
#include <vector>
#include <xtensor/xtensor.hpp>
#include <xtensor/xbuilder.hpp>

namespace corsika {

  /**
   * An implementation of a time-domain antenna that has a customized
   * start time, sampling period, and waveform duration.
   *
   */
  class TimeDomainAntenna final : public Antenna<TimeDomainAntenna> {

    TimeType const start_time_;      ///< The start time of this waveform.
    TimeType const duration_;        ///< The duration of this waveform.
    InverseTimeType const sample_rate_; ///< The sampling rate of this antenna.
    int num_bins_;                   ///< The number of bins used.
    xt::xtensor<double,2> waveformE_; ///< The waveform stored by this antenna.
    xt::xtensor<double,1> waveformT_;///< This xtensor stores timebins.
    std::pair<xt::xtensor<double, 1>, xt::xtensor<double,2>> waveform_; ///< E & t pair

  protected:
    // expose the CRTP interfaces constructor

  public:
    // import the methods from the antenna
    using Antenna<TimeDomainAntenna>::Antenna;
    using Antenna<TimeDomainAntenna>::receive;
    using Antenna<TimeDomainAntenna>::getName;
    using Antenna<TimeDomainAntenna>::getLocation;

    /**
     * Construct a new TimeDomainAntenna.
     *
     * @param name               The name of this antenna.
     * @param location           The location of this antenna.
     * @param start_time         The starting time of this waveform.
     * @param duration           The duration of this waveform.
     * @param sample_rate        The sample rate of this waveform.
     *
     */
    TimeDomainAntenna(std::string const& name, Point const& location,
                      TimeType const& start_time,
                      TimeType const& duration,
                      InverseTimeType const& sample_rate)
        : Antenna(name, location)
        , start_time_(start_time)
        , duration_(duration)
        , sample_rate_(sample_rate)
        , num_bins_ (static_cast<int>(duration * sample_rate))
        , waveformE_ (xt::zeros<double>({3, num_bins_}))
        , waveformT_(xt::zeros<double>({num_bins_}))
    {};

    /**
     * Receive an electric field at this antenna.
     *
     * This assumes that the antenna will receive
     *  an *instantaneous* electric field modeled as a delta function.
     *
     * @param time        The (global) time at which this
     * @param field       The incident electric field vector.
     *
     */
    void receive(TimeType const time, Vector const& receive_vector,
                 ElectricFieldVector const& efield) {

      if (time < start_time_ || time > start_time_ + duration_) {
        return;
      } else {

        auto timebin_ {static_cast<std::size_t>((time - start_time_) * sample_rate_)};

        for (std::size_t t = static_cast<std::size_t>(start_time_ / 1_ns);
             t < static_cast<std::size_t>((start_time_ + duration_) / 1_ns);
             t = t + timebin_) {
          waveformT_.at(t) = t; // store the sample times
        }

        // store the x,y,z electric field components
        waveformE_.at(0, timebin_) = waveformE_.at(0, timebin_) + (efield.getX().magnitude());
        waveformE_.at(1, timebin_) = waveformE_.at(1, timebin_) + (efield.getY().magnitude());
        waveformE_.at(2, timebin_) = waveformE_.at(2, timebin_) + (efield.getZ().magnitude());

        // create a std::pair of sample times & electric field components
        waveform_.first = waveformT_;
        waveform_.second = waveformE_;
      }
    }

    /**
     * Get the current waveform for this antenna.
     *
     * NOTE: Currently returns ns and V/m but this should be UNITful
     *
     * @returns A pair of the sample times, and the field
     */
    std::pair<xt::xtensor<double, 2>, xt::xtensor<double,1>> getWaveform() const {
      return waveform_;
    };

    /**
     * Reset the antenna before starting a new simulation.
     */
    void reset() {
      waveform_.first = xt::zeros_like(waveform_.first);
      waveform_.second = xt::zeros_like(waveform_.second);
    };

  }; // END: class Antenna final

} // namespace corsika
