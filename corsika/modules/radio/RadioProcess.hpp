/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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

        //    using ParticleType = corsika::setup::Stack::particle_type;
        //    using TrackType = corsika::LeapFrogTrajectory;

        /**
         * A collection of filter objects for deciding on valid particles and tracks.
         */
        // std::vector<std::function<bool(ParticleType&, TrackType const&)>> filters_;

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
        TAntennaCollection& antennas_; ///< The radio antennas we store into.
        TPropagator propagator_;       ///< The propagator implementation.
        int event_{0};                 ///< The current event ID.

    public:
        /**
         * Construct a new RadioProcess.
         */
        template <typename... TArgs>
        RadioProcess(TAntennaCollection& antennas, TArgs&&... args)
                : antennas_(antennas)
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
        ProcessReturn doContinuous(Particle const& particle, Track const& track, bool const) {
            // we want the following particles:
            // Code::Electron & Code::Positron & Code::Gamma

            // we wrap Simulate() in doContinuous as the plan is to add particle level
            // filtering or thinning for calculation of the radio emission. This is
            // important for controlling the runtime of radio (by ignoring particles
            // that aren't going to contribute i.e. heavy hadrons)
            // if (valid(particle, track)) {
            auto const particleID_ {particle.getPID()};
            if ((particleID_ == Code::Electron) || (particleID_ == Code::Positron)) {
                CORSIKA_LOG_DEBUG("Particle for radio calculation: {} ", particleID_);
                return this->implementation().simulate(particle, track);
            } else {
                CORSIKA_LOG_DEBUG("Particle {} is irrelevant for radio", particleID_);
                return ProcessReturn::Ok;
            }
            //}
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
        LengthType getMaxStepLength(Particle const& vParticle,
                                    Track const& vTrack) const {
            return meter * std::numeric_limits<double>::infinity();
        }

        /**
         * Called at the start of each library.
         */
        void startOfLibrary(boost::filesystem::path const& directory) final override {

            // loop over every antenna and set the initial path
            // this also writes the time-bins to disk.
            for (auto& antenna : antennas_.getAntennas()) { antenna.startOfLibrary(directory,this->implementation().algorithm);}
        }

        /**
         * Called at the end of each shower.
         */
        virtual void endOfShower(unsigned int const) final override {

            // loop over every antenna and instruct them to
            // flush data to disk, and then reset the antenna
            // before the next event
            for (auto& antenna : antennas_.getAntennas()) {
                antenna.endOfShower(event_, this->implementation().algorithm, antenna.sample_rate_*1_s);
                antenna.reset();
            }

            // increment our event counter
            event_++;
        }

        /**
         * Called at the end of each library.
         *
         */
        void endOfLibrary() final override {}

        /**
         * Get the configuration of this output.
         */
        YAML::Node getConfig() const final {

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

    }; // END: class RadioProcess

} // namespace corsika