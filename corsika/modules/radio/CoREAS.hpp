/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/RadioProcess.hpp>
#include <corsika/modules/radio/propagators/StraightPropagator.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  /**
   * 
   */
  template <typename TPropagator = StraightPropagator>
  class CoREAS final : public RadioProcess<CoREAS> {


  public:
    /**
     * Simulate the radio emission from a particle across a track.
     *
     * This must be provided by the TRadioImpl.
     *
     * @param particle    The current particle.
     * @param track       The current track.
     *
     */
    template <typename Particle, typename Track>
    EProcessReturn simulate(Particle&, Track const&) const {

        // loop over every antenna
        for (auto& antenna : getAntennas()) {

            // use TPropagator to calculate Path from track to antenna
            auto paths = ;/*  a collection of paths */

            // do endpoint + ZHS formalism

            // give the electric field and the direction to the antenna
            antenna.receive(...); // << something
        }

    }

    /**
     * Return the maximum step length for this particle and track.
     *
     * This must be provided by the TRadioImpl.
     *
     * @param particle    The current particle.
     * @param track       The current track.
     *
     * @returns The maximum length of this track.
     */
    template <typename Particle, typename Track>
    LengthType MaxStepLength(Particle const& particle,
                                        Track const& track) const;

  }; // END: class RadioProcess

} // namespace corsika
