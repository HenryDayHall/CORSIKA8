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
  template <typename... TArgs>
  inline RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::RadioProcess(
      TAntennaCollection& antennas, TArgs&&... args)
      : antennas_(antennas)
      , propagator_(args...) {}

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  template <typename Particle, typename Track>
  inline ProcessReturn RadioProcess<TAntennaCollection, TRadioImpl,
                                    TPropagator>::doContinuous(const Particle& particle,
                                                               const Track& track,
                                                               const bool) {
    // we want the following particles:
    // Code::Electron & Code::Positron & Code::Gamma

    // we wrap Simulate() in doContinuous as the plan is to add particle level
    // filtering or thinning for calculation of the radio emission. This is
    // important for controlling the runtime of radio (by ignoring particles
    // that aren't going to contribute i.e. heavy hadrons)
    // if (valid(particle, track)) {
    auto const particleID_{particle.getPID()};
    if ((particleID_ == Code::Electron) || (particleID_ == Code::Positron)) {
      CORSIKA_LOG_DEBUG("Particle for radio calculation: {} ", particleID_);
      return this->implementation().simulate(particle, track);
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

    // loop over every antenna and set the initial path
    // this also writes the time-bins to disk.
    for (auto& antenna : antennas_.getAntennas()) {
      antenna.startOfLibrary(directory, this->implementation().algorithm);
    }
  }

  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  inline void RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>::endOfShower(
      const unsigned int) {

    // loop over every antenna and instruct them to
    // flush data to disk, and then reset the antenna
    // before the next event
    for (auto& antenna : antennas_.getAntennas()) {
      antenna.endOfShower(event_, this->implementation().algorithm,
                          antenna.getSampleRate() * 1_s);
      antenna.reset();
    }

    // increment our event counter
    event_++;
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