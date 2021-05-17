/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/output/BaseOutput.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>

namespace corsika {

  class TrackWriterOff : public BaseOutput {

  public:
    /**
     * Construct a new writer.
     *
     * @param name    The name of this output.
     */
    TrackWriterOff() = default;

    /**
     * Called at the start of each library.
     */
    void startOfLibrary(boost::filesystem::path const&) final override {}

    /**
     * Called at the start of each shower.
     */
    void startOfShower(unsigned int const) final override {}

    /**
     * Called at the end of each shower.
     */
    void endOfShower(unsigned int const) final override {}

    /**
     * Called at the end of each library.
     *
     * This must also increment the run number since we override
     * the default behaviour of BaseOutput.
     */
    void endOfLibrary() final override {}

    /**
     * Write a track to the file.
     */
    void write(Code const& pid, units::si::HEPEnergyType const& energy,
               double const weight, QuantityVector<length_d> const& start,
               QuantityVector<length_d> const& end) {}

  }; // class TrackWriterOff

} // namespace corsika
