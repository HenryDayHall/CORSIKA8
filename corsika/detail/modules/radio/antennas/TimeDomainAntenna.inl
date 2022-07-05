/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>

namespace corsika {

  inline TimeDomainAntenna::TimeDomainAntenna(std::string const& name,
                                              Point const& location,
                                              CoordinateSystemPtr coordinateSystem,
                                              TimeType const& start_time,
                                              TimeType const& duration,
                                              InverseTimeType const& sample_rate,
                                              TimeType const ground_hit_time)
      : Antenna(name, location, coordinateSystem)
      , start_time_(start_time)
      , duration_(duration)
      , sample_rate_(sample_rate)
      , ground_hit_time_(ground_hit_time)
      , num_bins_(static_cast<std::size_t>(duration * sample_rate + 1.5l))
      , waveformEX_(num_bins_, 0)
      , waveformEY_(num_bins_, 0)
      , waveformEZ_(num_bins_, 0)
      , time_axis_(createTimeAxis()) {};

  inline void TimeDomainAntenna::receive(const TimeType time,
                                         const Vector<dimensionless_d>& receive_vector,
                                         const ElectricFieldVector& efield) {

    if (time < start_time_ || time > (start_time_ + duration_)) {
      return;
    } else {
      // figure out the correct timebin to store the E-field value.
      // NOTE: static cast is implicitly flooring
      auto timebin_{static_cast<std::size_t>(
          std::floor((time - start_time_) * sample_rate_ + 0.5l))};

      // store the x,y,z electric field components.
      auto const& Electric_field_components{efield.getComponents(coordinateSystem_)};
      waveformEX_.at(timebin_) += (Electric_field_components.getX() * (1_m / 1_V));
      waveformEY_.at(timebin_) += (Electric_field_components.getY() * (1_m / 1_V));
      waveformEZ_.at(timebin_) += (Electric_field_components.getZ() * (1_m / 1_V));
      // TODO: Check how they are stored in memory, row-wise or column-wise? Probably use a 3D object
    }
  }

  inline void TimeDomainAntenna::receive(const TimeType time,
                                         const Vector<dimensionless_d>& receive_vector,
                                         const VectorPotential& vectorP) {

    if (time < start_time_ || time > (start_time_ + duration_)) {
      return;
    } else {
      // figure out the correct timebin to store the E-field value.
      // NOTE: static cast is implicitly flooring
      auto timebin_{static_cast<std::size_t>(
          std::floor((time - start_time_) * sample_rate_ + 0.5l))};

      // store the x,y,z electric field components.
      auto const& Vector_potential_components{vectorP.getComponents(coordinateSystem_)};
      waveformEX_.at(timebin_) += (Vector_potential_components.getX() * (1_m / (1_V * 1_s)));
      waveformEY_.at(timebin_) += (Vector_potential_components.getY() * (1_m / (1_V * 1_s)));
      waveformEZ_.at(timebin_) += (Vector_potential_components.getZ() * (1_m / (1_V * 1_s)));
      // TODO: Check how they are stored in memory, row-wise or column-wise? Probably use a 3D object
    }
  }

  inline auto const& TimeDomainAntenna::getWaveformX() const { return waveformEX_; }

  inline auto const& TimeDomainAntenna::getWaveformY() const { return waveformEY_; }

  inline auto const& TimeDomainAntenna::getWaveformZ() const { return waveformEZ_; }

  inline std::vector<long double> TimeDomainAntenna::createTimeAxis() const {

    // create a 1-D xtensor to store time values so we can print them later.
    std::vector<long double> times(num_bins_, 0);

    // calculate the sample_period
    auto sample_period{1 / sample_rate_};

    // fill in every time-value
    for (std::size_t i = 0; i < num_bins_; i++) {
      // create the current time in nanoseconds
      times.at(i) = static_cast<long double>(
          ((start_time_ - ground_hit_time_) + i * sample_period) / 1_ns);
    }

    return times;
  }

  inline auto const& TimeDomainAntenna::getAxis() const { return time_axis_; }

  inline void TimeDomainAntenna::reset() {
    std::fill(waveformEX_.begin(), waveformEX_.end(), 0);
    std::fill(waveformEY_.begin(), waveformEY_.end(), 0);
    std::fill(waveformEZ_.begin(), waveformEZ_.end(), 0);
  }

  inline YAML::Node TimeDomainAntenna::getConfig() const {

    // top-level config
    YAML::Node config;

    config["type"] = "TimeDomainAntenna";
    config["start_time"] = start_time_ / 1_ns;
    config["duration"] = duration_ / 1_ns;
    config["sample_rate"] = sample_rate_ / 1_GHz;

    return config;
  }

} // namespace corsika