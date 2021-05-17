/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/output/BaseOutput.hpp>
#include <corsika/output/ParquetStreamer.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/media/ShowerAxis.hpp>

#include <vector>
#include <array>

namespace corsika {

  class EnergyLossWriterParquet : public BaseOutput {

    enum class ProfileIndex { Total, Entries };
    typedef std::array<HEPEnergyType, static_cast<int>(ProfileIndex::Entries)> Profile;

  public:
    /**
     * Construct a new writer.
     */
    EnergyLossWriterParquet(
        ShowerAxis const& axis,
        GrammageType dX = 10_g / square(1_cm),                // profile binning
        unsigned int const nBins = 200,                       // number of bins
        GrammageType dX_threshold = 0.0001_g / square(1_cm)); // ignore too short tracks

    /**
     * Called at the start of each library.
     */
    void startOfLibrary(boost::filesystem::path const& directory) final override;

    /**
     * Called at the start of each shower.
     */
    void startOfShower(unsigned int const showerId) final override;

    /**
     * Called at the end of each shower.
     */
    void endOfShower(unsigned int const showerId) final override;

    /**
     * Called at the end of each library.
     *
     * This must also increment the run number since we override
     * the default behaviour of BaseOutput.
     */
    void endOfLibrary() final override;

    /**
     * Add continuous energy loss.
     */
    template <typename TTrack>
    void write(TTrack const& track, Code const PID, HEPEnergyType const dE);

    /**
     * Add localized energy loss.
     */
    void write(Point const& point, Code const PID, HEPEnergyType const dE);

    /**
     * Add binned energy loss.
     */
    void write(GrammageType const Xstart, GrammageType const Xend,
               HEPEnergyType const dE);

    /**
     * Get total observed energy loss.
     *
     * @return HEPEnergyType The total energy.
     */
    HEPEnergyType getTotal() const;

    /**
     * Returns library-wide summary.
     */
    YAML::Node getSummary() const;

  public:
    ParquetStreamer output_; ///< The primary output file.

    ShowerAxis const& showerAxis_; ///< conversion between geometry and grammage
    GrammageType dX_;              ///< binning of profile.
    unsigned int nBins_;           ///< number of profile bins.
    GrammageType dX_threshold_;    ///< too short tracks are discarded.
    std::vector<Profile> profile_; // longitudinal profile

  }; // namespace corsika

} // namespace corsika

#include <corsika/detail/modules/writers/EnergyLossWriterParquet.inl>
