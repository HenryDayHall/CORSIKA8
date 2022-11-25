/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
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
    void addAntenna(TAntennaImpl const& antenna);

    /**
     * Get the specific antenna at that place in the collection
     *
     * @param index in the collection
     */
    TAntennaImpl& at(std::size_t const i);

    TAntennaImpl const& at(std::size_t const i) const;

    /**
     * Get the number of antennas in the collection
     */
    int size() const;

    /**
     * Get a *non*-const reference to the collection of antennas.
     *
     * @returns    An iterable mutable reference to the antennas.
     */
    std::vector<TAntennaImpl>& getAntennas();

    /**
     * Get a const reference to the collection of antennas.
     *
     * @returns    An iterable mutable reference to the antennas.
     */
    std::vector<TAntennaImpl> const& getAntennas() const;

    /**
     * Reset all the antenna waveforms.
     */
    void reset();
  }; // END: class RadioDetector

} // namespace corsika

#include <corsika/detail/modules/radio/detectors/AntennaCollection.inl>
