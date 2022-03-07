/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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
    std::vector<double> waveformEX_;    ///< EX polarization
    std::vector<double> waveformEY_;    ///< EY polarization
    std::vector<double> waveformEZ_;    ///< EZ polarization
    TimeType const ground_hit_time_;      ///< The time the primary particle hits the ground.

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
                      InverseTimeType const& sample_rate, TimeType const& ground_hit_time);

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
                 ElectricFieldVector const& efield);

      void receive(TimeType const time, Vector<dimensionless_d> const& receive_vector,
                   VectorPotential const& vectorP);

    /**
     * Return the time-units of each waveform for X polarization
     *
     * This returns them in nanoseconds for ease of use.
     */
    auto& getDataX() const;

    /**
     * Return the time-units of each waveform for Y polarization
     *
     * This returns them in nanoseconds for ease of use.
     */
    auto& getDataY() const;

    /**
     * Return the time-units of each waveform for Z polarization
     *
     * This returns them in nanoseconds for ease of use.
     */
    auto& getDataZ() const;

    /**
     * Return the time-units of each waveform.
     *
     * This returns them in nanoseconds for ease of use.
     */
    auto getAxis() const;

    // TODO: These should get deleted or renamed to something more sensible
    auto getWaveformX() const;

    auto getWaveformY() const;

    auto getWaveformZ() const;

    /**
     * Reset the antenna before starting a new simulation.
     */
      void reset();

    /**
     * Return a YAML configuration for this antenna.
     */
    YAML::Node getConfig() const;

  }; // END: class TimeDomainAntenna

} // namespace corsika

#include <corsika/detail/modules/radio/antennas/TimeDomainAntenna.inl>