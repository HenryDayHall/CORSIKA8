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

namespace corsika {

  /**
   * A concrete implementation of the ZHS algorithm.
   */
  template <typename TRadioDetector, typename TPropagator>
  class ZHS final : public RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator> {

      using Base = RadioProcess<TRadioDetector, ZHS<TRadioDetector, TPropagator>, TPropagator>;
      using Base::detector_;
      
  public:
    /**
     * Construct a new ZHS instance.
     *
     * This forwards the detector and other arguments to
     * the RadioProcess parent.
     *
     */
    template <typename... TArgs>
    ZHS(TRadioDetector& detector, TArgs&&... args)
        : RadioProcess<TRadioDetector, ZHS, TPropagator>(detector, args...){}

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
    ProcessReturn simulate(Particle& particle, Track const& track) const {

      auto global_time = particle.getTime(); // this is very shady at the moment...
      //get global time for that track
      auto starttime = track.getDuration(0); // time at start point of track.
      auto endtime = track.getDuration(1); // time at end point of track.

      // we loop over each antenna in the collection
      for (auto& antenna : detector_.getAntennas()) {

        // auto start = /* TODO: get Point from Track */;
        auto start = track.getStart(); // just another shady idea...

        // get the Path from the track to the antenna
        // This is a SignalPathCollection
        auto paths{this->propagator_.propagate(start, antenna.getLocation())};

        // now loop over the paths that we got above
        // Note: for the StraightPropagator, there will only be a single
        // path but other propagators may return more than one.
        for (auto const& path : paths) {
//          path.total_time_ + global_time;
//          path.average_refractivity_;
//          path.emit_;
//          path.receive_;

          // calculate the ZHS formalism for this particle-antenna
          // combination along this path.
          // global time + time delay
          // and pass it to the antenna.
          antenna.receive(/* global time + time delay, receive vector, ElectricFieldVector */);

        } // END: loop over paths

      } // END: loop over antennas
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
                                        Track const& track) const {

      // TODO : This is where te control the maximum step size
      // of a particle track in order to maintain the accuracy
      // of the particular formalism.
      //
      // This is part of the ZHS / CoReas formalisms and can
      // be related from the magnetic field / acceleration, charge,
      // etc. of the particle.
    }

  }; // END: class RadioProcess

} // namespace corsika
