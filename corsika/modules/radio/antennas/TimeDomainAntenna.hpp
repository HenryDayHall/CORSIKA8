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

namespace corsika {

  /**
   * An implementation of a time-domain antenna that has a customized
   * start time, sampling period, and waveform duration.
   *
   */
  class TimeDomainAntenna final : public Antenna<TimeDomainAntenna> {

    using Array = std::vector<double>; ///< The type used to store arrays.

    TimeType const start_time_;      ///< The start time of this waveform.
    TimeType const duration_;        ///< The duration of this waveform.
    TimeType const sampling_period_; ///< The sampling period of this antenna.

//    std::pair<Array, Array> waveformX_; // time + Ex component (?)
//    std::pair<Array, Array> waveformY_; // time + Ey component (?)
//    std::pair<Array, Array> waveformZ_; // time + Ez component (?)
    std::pair<Array, Array> waveform_; ///< The waveform stored by this antenna. This confuses me a lot. The first should be time and second E field?

  protected:
    // expose the CRTP interfaces constructor

  public:
    // import the methods from the antenna
    using Antenna<TimeDomainAntenna>::Antenna;
    using Antenna<TimeDomainAntenna>::getName;

    /**
     * Construct a new TimeDomainAntenna.
     *
     * @param name               The name of this antenna.
     * @param location           The location of this antenna.
     * @param start_time         The starting time of this waveform.
     * @param duration           The duration of this waveform.
     * @param sampling_period    The sampling period of this waveform.
     *
     */
    TimeDomainAntenna(std::string const& name, Point const& location,
                      TimeType const& start_time,
                      TimeType const& duration,
                      TimeType const& sampling_period)
        : Antenna(name, location)
        , start_time_(start_time)
        , duration_(duration)
        , sampling_period_(sampling_period){};

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
    void receive(TimeType const time, ElectricFieldVector const& efield) const {

      if (time < start_time_ || time > start_time_ + duration_) {
        return;
      } else {
        auto num_bins_ = static_cast<int>(duration_ / sampling_period_);
        Array timebins_ (num_bins_,0);
        auto timebin_ {(time - start_time_) / sampling_period_ };
        timebins_.at(timebin_) = timebin_; //for sure this is not going to work
      }

    }

    /**
     * Get the current waveform for this antenna.
     *
     * NOTE: Currently returns ns and V/m but this should be UNITful
     *
     * @returns A pair of the sample times, and the field
     */
    std::pair<Array, Array> getWaveform() const {
      return waveform_;
    };

    /**
     * Reset the antenna before starting a new simulation.
     */
    void reset() {
      waveform_.first.clear();
      waveform_.second.clear();
    };

  }; // END: class Antenna final

} // namespace corsika
