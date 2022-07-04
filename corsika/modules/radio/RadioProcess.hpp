/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <istream>
#include <fstream>
#include <iostream>
#include <string>
#include <corsika/output/BaseOutput.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika {

  /**
   * The base interface for radio emission processes.
   *
   * TRadioImpl is the concrete implementation of the radio algorithm.
   * TAntennaCollection is the detector instance that stores antennas
   * and is responsible for managing the output writing.
   */
  template <typename TAntennaCollection, typename TRadioImpl, typename TPropagator>
  class RadioProcess : public ContinuousProcess<
                           RadioProcess<TAntennaCollection, TRadioImpl, TPropagator>>,
                       public BaseOutput {

    /*
     * A collection of filter objects for deciding on valid particles and tracks.
     */
    // std::vector<std::function<bool(ParticleType&, TrackType const&)>> filters_;

    /**
     * Get a reference to the underlying radio implementation.
     */
    TRadioImpl& implementation();

    /**
     *  Get a const reference to the underlying implementation.
     */
    TRadioImpl const& implementation() const;

  protected:
    TAntennaCollection& antennas_; ///< The radio antennas we store into.
    TPropagator propagator_;       ///< The propagator implementation.
    int event_{0};                 ///< The current event ID.

  public:
    /**
     * Construct a new RadioProcess.
     */
    template <typename... TArgs>
    RadioProcess(TAntennaCollection& antennas, TArgs&&... args);

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
    ProcessReturn doContinuous(Particle const& particle, Track const& track, bool const);

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
    LengthType getMaxStepLength(Particle const& vParticle, Track const& vTrack) const;

    /**
     * Called at the start of each library.
     */
    void startOfLibrary(boost::filesystem::path const& directory) final override;

    /**
     * Called at the end of each shower.
     */
    virtual void endOfShower(unsigned int const) final override;

    /**
     * Called at the end of each library.
     *
     */
    void endOfLibrary() final override {}

    /**
     * Get the configuration of this output.
     */
    YAML::Node getConfig() const final;

  }; // END: class RadioProcess

} // namespace corsika

#include <corsika/detail/modules/radio/RadioProcess.inl>