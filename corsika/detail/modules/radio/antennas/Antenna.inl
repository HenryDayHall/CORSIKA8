/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/antennas/Antenna.hpp>

namespace corsika {

  template <typename TAntennaImpl>
  inline Antenna<TAntennaImpl>::Antenna(std::string const& name, Point const& location,
                                        CoordinateSystemPtr const& coordinateSystem)
      : name_(name)
      , location_(location)
      , coordinateSystem_(coordinateSystem) {}

  template <typename TAntennaImpl>
  inline Point const& Antenna<TAntennaImpl>::getLocation() const {
    return location_;
  }

  template <typename TAntennaImpl>
  inline std::string const& Antenna<TAntennaImpl>::getName() const {
    return name_;
  }

  template <typename TAntennaImpl>
  inline TAntennaImpl& Antenna<TAntennaImpl>::implementation() {
    return static_cast<TAntennaImpl&>(*this);
  }

} // namespace corsika
