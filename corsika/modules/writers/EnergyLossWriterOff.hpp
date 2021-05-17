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
#include <corsika/media/ShowerAxis.hpp>

namespace corsika {

  class EnergyLossWriterOff : public BaseOutput {

  public:
    /**
     * Construct a new writer.
     */
    EnergyLossWriterOff() = default;

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
    template <typename TTrack>
    void write(TTrack const&, Code const, HEPEnergyType const) {}

    /**
     * Add localized energy loss.
     */
    void write(Point const&, Code const, HEPEnergyType const) {}

    /**
     * Add binned energy loss.
     */
    void write(GrammageType const, GrammageType const, HEPEnergyType const) {}

    HEPEnergyType getTotal() const { return 0_GeV; }

  }; // namespace corsika

} // namespace corsika
