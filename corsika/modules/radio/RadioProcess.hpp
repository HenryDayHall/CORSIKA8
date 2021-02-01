/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/framework/process/ContinuousProcess.hpp>

namespace corsika {

  /**
   * The base interface for radio emission processes.
   *
   * TRadioImpl is the concrete implementation of the radio algorithm.
   * TRadioDetector is the detector instance that stores antennas
   * and is responsible for managing the output writing.
   */
  template <typename TRadioDetector, typename TRadioImpl, typename TPropagator>
  class RadioProcess : public ContinuousProcess<
                           RadioProcess<TRadioDetector, TRadioImpl, TPropagator>> {

    /**
     * Get a reference to the underlying radio implementation.
     */
    TRadioImpl& implementation() { return static_cast<TRadioImpl&>(*this); }

    /*
     *  Get a const reference to the underlying implementation.
     */
    TRadioImpl const& implementation() const {
      return static_cast<TRadioImpl const&>(*this);
    }

  protected:
    TRadioDetector& detector_; ///< The radio detector we store into.
    TPropagator propagator_;   ///< The propagator implementation.

  public:
    /**
     * Construct a new RadioProcess.
     */
    template <typename... TArgs>
    RadioProcess(TRadioDetector& detector, TArgs&&... args)
        : detector_(detector)
        , propagator_(args...) {}

    /**
     * Perform the continuous process (radio emission).
     *
     * This handles filtering individual particle tracks
     * before passing them to `Simulate`.`
     *
     * @param particle    The current particle.
     * @param track       The current track.
     */
    template <typename Particle, typename Track>
    ProcessReturn DoContinuous(Particle& particle, Track const& track) const {
      // we wrap Simulate() in DoContinuous as the plan is to add particle level
      // filtering or thinning for calculation of the radio emission. This is
      // important for controlling the runtime of radio (by ignoring particles
      // that aren't going to contribute i.e. heavy hadrons)
      return this->implementation().simulate(particle, track);
    }

      /**
       * TODO: This is placeholder so we can use text output while
       * we wait for the true output formatting to be ready.
       **/
      bool writeOutput() const {

        // how this method should work:
        // 1. Loop over the antennas in the collection
        // 2. Get their waveforms
        // 3. Create a textfile for each antenna
        // 4. and write out two columns, time and field.

      }

  }; // END: class RadioProcess

} // namespace corsika::process::radio
