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

#include <corsika/modules/radio/antennas/TimeDomainAntenna.hpp>

namespace corsika {

    inline TimeDomainAntenna::TimeDomainAntenna(const std::string &name, const Point &location,
                                                const TimeType &start_time, const TimeType &duration,
                                                const InverseTimeType &sample_rate, const TimeType &ground_hit_time)
            : Antenna(name, location)
            , start_time_(start_time)
            , duration_(duration)
            , sample_rate_(sample_rate)
            , ground_hit_time_(ground_hit_time)
            , num_bins_(static_cast<std::size_t>(duration * sample_rate + 1.5l))
            , waveformEX_(num_bins_, 0)
            , waveformEY_(num_bins_, 0)
            , waveformEZ_(num_bins_, 0) {};

    inline void TimeDomainAntenna::receive(const TimeType time, const Vector<dimensionless_d> &receive_vector,
                                           const ElectricFieldVector &efield) {

        if (time < start_time_ || time > (start_time_ + duration_)) {
            return;
        } else {
            // figure out the correct timebin to store the E-field value.
            // NOTE: static cast is implicitly flooring
            auto timebin_{static_cast<std::size_t>(std::floor((time - start_time_) * sample_rate_ + 0.5l))};
//            CORSIKA_LOG_DEBUG("Timebin: {}", timebin_);

            // ToDO: ask explicitly for a CS and use that specific on for writing the output

            // store the x,y,z electric field components.
            waveformEX_.at(timebin_) += (efield.getComponents().getX() / (1_V / 1_m));
            waveformEY_.at(timebin_) += (efield.getComponents().getY() / (1_V / 1_m));
            waveformEZ_.at(timebin_) += (efield.getComponents().getZ() / (1_V / 1_m));
            // TODO: Check how they are stored in memory, row-wise or column-wise?
        }
    }

    inline void TimeDomainAntenna::receive(const TimeType time, const Vector<dimensionless_d> &receive_vector,
                                           const VectorPotential &vectorP) {

        if (time < start_time_ || time > (start_time_ + duration_)) {
            return;
        } else {
            // figure out the correct timebin to store the E-field value.
            // NOTE: static cast is implicitly flooring
            auto timebin_{static_cast<std::size_t>(std::floor((time - start_time_) * sample_rate_ + 0.5l))};
//            CORSIKA_LOG_DEBUG("Timebin: {}", timebin_);

            // ToDO: ask explicitly for a CS and use that specific on for writing the output

            // store the x,y,z electric field components.
            waveformEX_.at(timebin_) += (vectorP.getComponents().getX() / (1_V * 1_s / 1_m));
            waveformEY_.at(timebin_) += (vectorP.getComponents().getY() / (1_V * 1_s / 1_m));
            waveformEZ_.at(timebin_) += (vectorP.getComponents().getZ() / (1_V * 1_s / 1_m));
            // TODO: Check how they are stored in memory, row-wise or column-wise?
        }
    }

    inline auto& TimeDomainAntenna::getDataX() const { return waveformEX_; }

    inline auto& TimeDomainAntenna::getDataY() const { return waveformEY_; }

    inline auto& TimeDomainAntenna::getDataZ() const { return waveformEZ_; }

    inline auto TimeDomainAntenna::getAxis() const {

        // create a 1-D xtensor to store time values so we can print them later.
        std::vector<long double> times(num_bins_, 0);

        // calculate the sample_period
        auto sample_period{1 / sample_rate_};

        // fill in every time-value
        // TODO: Vectorize this
        for (std::size_t i = 0; i < num_bins_; i++) {
            // create the current time in nanoseconds
            times.at(i) = static_cast<long double>(((start_time_ - ground_hit_time_) + i*sample_period) / 1_ns);
        }

        return times;
    }

    inline auto TimeDomainAntenna::getWaveformX() const { return std::make_pair(getAxis(), waveformEX_); }

    inline auto TimeDomainAntenna::getWaveformY() const { return std::make_pair(getAxis(), waveformEY_); }

    inline auto TimeDomainAntenna::getWaveformZ() const { return std::make_pair(getAxis(), waveformEZ_); }

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