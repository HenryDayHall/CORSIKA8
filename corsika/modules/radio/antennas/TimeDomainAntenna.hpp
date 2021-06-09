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
   * start time, sampling rate, and waveform duration.
   *
   */
  class TimeDomainAntenna : public Antenna<TimeDomainAntenna> {

  public:
    // import the methods from the antenna

    // label this as a time-domain antenna.
    static constexpr bool is_time_domain{true};

    TimeType const start_time_;         ///< The start time of this waveform.
    TimeType const duration_;           ///< The duration of this waveform.
    InverseTimeType const sample_rate_; ///< The sampling rate of this antenna.
    int num_bins_;                      ///< The number of bins used.
    xt::xtensor<double, 2> waveformE_;  ///< The waveform stored by this antenna.

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
     * @param num_bins_          The number of timebins to store E-field.
     * @param waveformE_         The xtensor initialized to zero for E-field.
     *
     */
    TimeDomainAntenna(std::string const& name, Point const& location,
                      TimeType const& start_time, TimeType const& duration,
                      InverseTimeType const& sample_rate)
        : Antenna(name, location)
        , start_time_(start_time)
        , duration_(duration)
        , sample_rate_(sample_rate)
        , num_bins_(static_cast<std::size_t>(duration * sample_rate + 1.5l))
        , waveformE_(xt::zeros<double>({num_bins_, 3})){};

    /**
     * Receive an electric field at this antenna.
     *
     * This assumes that the antenna will receive
     *  an *instantaneous* electric field modeled as a delta function (or timebin).
     *
     * @param time             The (global) time at which this signal is received.
     * @param receive_vector   The incident unit vector. (not used at the moment)
     * @param field            The incident electric field vector.
     *
     */
    // TODO: rethink this method a bit. If the endpoint is at the end of the antenna
    // resolution then you get the startpoint signal but you lose the endpoint signal!
    void receive(TimeType const time, Vector<dimensionless_d> const& receive_vector,
                 ElectricFieldVector const& efield) {

      if (time < start_time_ || time > (start_time_ + duration_)) {
        return;
      } else {
        // figure out the correct timebin to store the E-field value.
        // NOTE: static cast is implicitly flooring
        auto timebin_{static_cast<std::size_t>(std::floor((time - start_time_) * sample_rate_ + 0.5l))};
        CORSIKA_LOG_INFO("Timebin: {}", timebin_);

        // ToDO: ask explicitly for a CS and use that specific on for writing the output

        // store the x,y,z electric field components.
        waveformE_.at(timebin_, 0) += (efield.getComponents().getX() / (1_V / 1_m));
        waveformE_.at(timebin_, 1) += (efield.getComponents().getY() / (1_V / 1_m));
        waveformE_.at(timebin_, 2) += (efield.getComponents().getZ() / (1_V / 1_m));
        // TODO: Check how they are stored in memory, row-wise or column-wise?
      }
    }

      void receive(TimeType const time, Vector<dimensionless_d> const& receive_vector,
                   VectorPotential const& vectorP) {

          if (time < start_time_ || time > (start_time_ + duration_)) {
              return;
          } else {
              // figure out the correct timebin to store the E-field value.
              // NOTE: static cast is implicitly flooring
              auto timebin_{static_cast<std::size_t>(std::floor((time - start_time_) * sample_rate_ + 0.5l))};
              CORSIKA_LOG_INFO("Timebin: {}", timebin_);

              // ToDO: ask explicitly for a CS and use that specific on for writing the output

              // store the x,y,z electric field components.
              waveformE_.at(timebin_, 0) += (vectorP.getComponents().getX() / (1_V * 1_s / 1_m));
              waveformE_.at(timebin_, 1) += (vectorP.getComponents().getY() / (1_V * 1_s / 1_m));
              waveformE_.at(timebin_, 2) += (vectorP.getComponents().getZ() / (1_V * 1_s / 1_m));
              // TODO: Check how they are stored in memory, row-wise or column-wise?
          }
      }

    /**
     * Return the time-units of each waveform.
     *
     * This returns them in nanoseconds for ease of use.
     */
    auto& getData() const { return waveformE_; }

    /**
     * Return the time-units of each waveform.
     *
     * This returns them in nanoseconds for ease of use.
     */
    auto getAxis() const {

      // create a 1-D xtensor to store time values so we can print them later.
      xt::xtensor<long double, 1> times(xt::zeros<long double>({num_bins_}));

      // calculate the sample_period
      auto sample_period{1 / sample_rate_};

      // fill in every time-value
      // TODO: Vectorize this using xtensor
      for (std::size_t i = 0; i < num_bins_; i++) {
        // create the current time in nanoseconds
        times.at(i) = static_cast<long double>((start_time_ + i*sample_period) / 1_ns);
      }

      return times;
    }

    // TODO: This should get deleted or renamed to something more sensible
    auto getWaveform() const { return std::make_pair(getAxis(), waveformE_); }

    /**
     * Reset the antenna before starting a new simulation.
     */
    void reset() { waveformE_ = xt::zeros_like(waveformE_); };

    /**
     * Return a YAML configuration for this antenna.
     */
    YAML::Node getConfig() const {

      // top-level config
      YAML::Node config;

      config["type"] = "TimeDomainAntenna";
      config["start_time"] = start_time_ / 1_ns;
      config["duration"] = duration_ / 1_ns;
      config["sample_rate"] = sample_rate_ / 1_GHz;

      return config;
    }

  }; // END: class Antenna final

} // namespace corsika
