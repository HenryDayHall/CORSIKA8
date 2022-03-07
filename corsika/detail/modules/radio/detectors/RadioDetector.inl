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

#include <corsika/modules/radio/detectors/RadioDetector.hpp>

namespace corsika {

  template <typename TAntennaImpl>
  inline AntennaCollection::AntennaCollection {
    std::vector<TAntennaImpl> antennas_;
  }

  template <typename TAntennaImpl>
  inline void AntennaCollection::addAntenna(TAntennaImpl const& antenna) {
    antennas_.push_back(antenna);
  }

  template <typename TAntennaImpl>
  inline TAntennaImpl AntennaCollection::at(std::size_t const i) {
    antennas_.at(i);
  }

  inline int AntennaCollection::size() { return antennas_.size(); }

  template <typename TAntennaImpl>
  inline std::vector<TAntennaImpl>& AntennaCollection::getAntennas() {
    return antennas_;
  }

  inline void AntennaCollection::reset() {
    std::for_each(antennas_.begin(), antennas_.end(), std::mem_fn(&TAntennaImpl::reset));
  }

} // namespace corsika
