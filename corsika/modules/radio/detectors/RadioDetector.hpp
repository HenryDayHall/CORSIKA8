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

namespace corsika {

  /**
   * The base interface for radio detectors.
   * At the moment it is a collection of antennas with the same implementation.
   */

  template <typename TAntennaImpl>
  class AntennaCollection {

    /**
     * The collection of antennas used in this simulation.
     */
    std::vector<TAntennaImpl> antennas_;

  public:
    /**
     * Add an antenna to this radio process.
     *
     * @param antenna    The antenna to add
     */
    void addAntenna(TAntennaImpl const antenna) { antennas_.push_back(antenna); }

    TAntennaImpl at(std::size_t const i) {antennas_.at(i);}

    /**
     * Get a *non*-const reference to the collection of antennas.
     *
     * @returns    An iterable mutable reference to the antennas.
     */
    std::vector<TAntennaImpl> const& getAntennas() { return antennas_; } // maybe the const& here is an issue

    /**
     * Reset all the antenna waveforms.
     */
    void reset() {
      std::for_each(antennas_.begin(), antennas_.end(), std::mem_fn(&TAntennaImpl::reset));
    };

  }; // END: class RadioDetector

} // namespace corsika
