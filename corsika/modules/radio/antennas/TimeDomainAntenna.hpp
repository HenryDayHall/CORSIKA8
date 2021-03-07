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


//  protected:
//    // expose the CRTP interfaces constructor

  public:
    // import the methods from the antenna

    TimeType const start_time_;      ///< The start time of this waveform.
    TimeType const duration_;        ///< The duration of this waveform.
    InverseTimeType const sample_rate_; ///< The sampling rate of this antenna.
    int num_bins_;                   ///< The number of bins used.
    xt::xtensor<double,2> waveformE_; ///< The waveform stored by this antenna.
    std::pair<xt::xtensor<double, 2>,
        xt::xtensor<double,2>> waveform_; ///< useful for .getWaveform()

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
                      TimeType const& start_time,
                      TimeType const& duration,
                      InverseTimeType const& sample_rate)
        : Antenna(name, location)
        , start_time_(start_time)
        , duration_(duration)
        , sample_rate_(sample_rate)
        , num_bins_ (static_cast<int>(duration * sample_rate))
        , waveformE_ (xt::zeros<double>({num_bins_, 3}))
    {};

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
    void receive(TimeType const time, Vector<dimensionless_d> const& receive_vector,
                 ElectricFieldVector const& efield) {

      if (time < start_time_ || time >= start_time_ + duration_) {
        return;
      } else {
        // figure out the correct timebin to store the E-field value.
        auto timebin_ {static_cast<std::size_t>((time - start_time_) * sample_rate_)};

        // store the x,y,z electric field components.
        waveformE_.at(timebin_, 0) += efield.getX().magnitude();
        waveformE_.at(timebin_, 1) += efield.getY().magnitude();
        waveformE_.at(timebin_, 2) += efield.getZ().magnitude();
        //TODO: Check how they are stored in memory, row-wise or column-wise?
      }
    }

    /**
     * Get the current waveform for this antenna.
     *
     * NOTE: Currently returns ns and V/m but this should be UNITful
     *
     * @returns A pair of the sample times, and the field
     */
    std::pair<xt::xtensor<double, 2>, xt::xtensor<double,2>> getWaveform() const {
      // TODO: divide by Δt for CoREAS ONLY!

      // create a 1-D xtensor to store time values so we can print them later.
      xt::xtensor<double, 2> times_ (xt::zeros<double>({num_bins_, 1}));

      for (int i = 0; i < num_bins_; i++) {
        times_.at(i,0) = static_cast<double>(start_time_ / 1_s + i * sample_rate_ * 1_s);
      }

      return std::make_pair(times_, waveformE_);
    };

    /**
     * Reset the antenna before starting a new simulation.
     */
    void reset() {
      waveformE_ = xt::zeros_like(waveformE_);
//      times_ = xt::zeros_like(times_);
    };

  }; // END: class Antenna final

} // namespace corsika
