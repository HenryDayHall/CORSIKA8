/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/RadioProcess.hpp>

namespace corsika {

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  inline TRadioImpl&
  RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::implementation() {
    return static_cast<TRadioImpl&>(*this);
  }

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  inline TRadioImpl const&
  RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::implementation() const {
    return static_cast<TRadioImpl const&>(*this);
  }

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  inline RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::RadioProcess(
      TAntennaCollection& antennas, TPropagator& propagator)
      : antennas_(antennas)
      , propagator_(propagator) {}

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  template <typename Particle>
  inline ProcessReturn RadioProcess<TAntennaCollection, TRadioImpl,
                                    TPropagator>::doContinuous(const Step<Particle>& step,
                                                               const bool) {
    // we want the following particles:
    // Code::Electron & Code::Positron

    // we wrap Simulate() in doContinuous as the plan is to add particle level
    // filtering or thinning for calculation of the radio emission. This is
    // important for controlling the runtime of radio (by ignoring particles
    // that aren't going to contribute i.e. heavy hadrons)
    // if (valid(step)) {
    auto const particleID_{step.getParticlePre().getPID()};
    if ((particleID_ == Code::Electron) || (particleID_ == Code::Positron)) {
      CORSIKA_LOG_DEBUG("Particle for radio calculation: {} ", particleID_);
      return this->implementation().simulate(step);
    } else {
      CORSIKA_LOG_DEBUG("Particle {} is irrelevant for radio", particleID_);
      return ProcessReturn::Ok;
    }
    //}
  }

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  template <typename Particle, typename Track>
  inline LengthType
  RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::getMaxStepLength(
      const Particle& vParticle, const Track& vTrack) const {
    return meter * std::numeric_limits<double>::infinity();
  }

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  inline void RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::startOfLibrary(
      const boost::filesystem::path& directory) {

    // setup the streamer
    output_.initStreamer((directory / ("antennas.parquet")).string());
    // LCOV_EXCL_START
    // build the schema
    output_.addField("Time", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
                     parquet::ConvertedType::NONE);

    output_.addField("Ex", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
                     parquet::ConvertedType::NONE);

    output_.addField("Ey", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
                     parquet::ConvertedType::NONE);

    output_.addField("Ez", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
                     parquet::ConvertedType::NONE);
    // LCOV_EXCL_STOP
    // and build the streamer
    output_.buildStreamer();
  }

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  inline void RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::endOfShower(
      const unsigned int) {

    // loop over every antenna and instruct them to
    // flush data to disk, and then reset the antenna
    // before the next event
    for (auto& antenna : antennas_.getAntennas()) {

      auto const sampleRate = antenna.getSampleRate() * 1_s;
      auto const radioImplementation =
          static_cast<std::string>(this->implementation().algorithm);

      // get the axis labels for this antenna and write the first row.
      axistype axis = antenna.implementation().getAxis();

      // get the copy of the waveform data for this event
      std::vector<double> const& dataX = antenna.implementation().getWaveformX();
      std::vector<double> const& dataY = antenna.implementation().getWaveformY();
      std::vector<double> const& dataZ = antenna.implementation().getWaveformZ();

      // check for the axis name
      std::string label = "Unknown";
      if (antenna.getDomainLabel() == "Time") {
        label = "Time";
      }
      // LCOV_EXCL_START
      else if (antenna.getDomainLabel() == "Frequency") {
        label = "Frequency";
      }
      // LCOV_EXCL_STOP
      if (radioImplementation == "ZHS" && label == "Time") {
        for (size_t i = 0; i < axis.size() - 1; i++) {
          auto time = (axis.at(i + 1) + axis.at(i)) / 2.;
          auto Ex = -(dataX.at(i + 1) - dataX.at(i)) * sampleRate;
          auto Ey = -(dataY.at(i + 1) - dataY.at(i)) * sampleRate;
          auto Ez = -(dataZ.at(i + 1) - dataZ.at(i)) * sampleRate;

          *(output_.getWriter())
              << showerId_ << static_cast<double>(time) << static_cast<double>(Ex)
              << static_cast<double>(Ey) << static_cast<double>(Ez) << parquet::EndRow;
        }
      } else if (radioImplementation == "CoREAS" && label == "Time") {
        for (size_t i = 0; i < axis.size() - 1; i++) {
          *(output_.getWriter())
              << showerId_ << static_cast<double>(axis[i])
              << static_cast<double>(dataX[i]) << static_cast<double>(dataY[i])
              << static_cast<double>(dataZ[i]) << parquet::EndRow;
        }
      }

      antenna.reset();
    }
    output_.closeStreamer();

    // increment our event counter
    showerId_++;
  }

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  inline YAML::Node RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::getConfig()
      const {

    // top-level YAML node
    YAML::Node config;

    // fill in some basics
    config["type"] = "RadioProcess";
    config["algorithm"] = this->implementation().algorithm;
    config["units"]["time"] = "ns";
    config["units"]["frequency"] = "GHz";
    config["units"]["electric field"] = "V/m";
    config["units"]["distance"] = "m";

    for (auto& antenna : antennas_.getAntennas()) {
      // get the name/location of this antenna
      auto name = antenna.getName();
      auto location = antenna.getLocation().getCoordinates();

      // get the antennas config
      config["antennas"][name] = antenna.getConfig();

      // write the location of this antenna
      config["antennas"][name]["location"].push_back(location.getX() / 1_m);
      config["antennas"][name]["location"].push_back(location.getY() / 1_m);
      config["antennas"][name]["location"].push_back(location.getZ() / 1_m);
    }

    return config;
  }

} // namespace corsika