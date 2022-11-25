/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/detectors/AntennaCollection.hpp>

namespace corsika {

  template <typename TAntennaImpl>
  inline void AntennaCollection<TAntennaImpl>::addAntenna(TAntennaImpl const& antenna) {
    antennas_.push_back(antenna);
  }

  template <typename TAntennaImpl>
  inline TAntennaImpl& AntennaCollection<TAntennaImpl>::at(std::size_t const i) {
    return antennas_.at(i);
  }

  template <typename TAntennaImpl>
  inline TAntennaImpl const& AntennaCollection<TAntennaImpl>::at(
      std::size_t const i) const {
    return antennas_.at(i);
  }

  template <typename TAntennaImpl>
  inline int AntennaCollection<TAntennaImpl>::size() const {
    return antennas_.size();
  }

  template <typename TAntennaImpl>
  inline std::vector<TAntennaImpl>& AntennaCollection<TAntennaImpl>::getAntennas() {
    return antennas_;
  }

  template <typename TAntennaImpl>
  inline void AntennaCollection<TAntennaImpl>::reset() {
    std::for_each(antennas_.begin(), antennas_.end(), std::mem_fn(&TAntennaImpl::reset));
  }

} // namespace corsika
