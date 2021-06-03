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
#include <string>

namespace corsika {

  class LongitudinalProfileWriterParquet : public BaseOutput {

    enum class ProfileIndex {
      Charged,
      Hadron,
      Photon,
      Electron,
      Positron,
      MuPlus,
      MuMinus,
      Entries
    };

    static std::array<
        char const*, static_cast<int>(ProfileIndex::Entries)> constexpr ProfileIndexNames{
        {"charged", "hadron", "photon", "electron", "positron", "muplus", "muminus"}};

    typedef std::array<double, static_cast<int>(ProfileIndex::Entries)> ProfileData;

  public:
    /**
     * Construct a new writer.
     */
    LongitudinalProfileWriterParquet(ShowerAxis const& axis,
                                     GrammageType dX = 10_g /
                                                       square(1_cm),  // profile binning
                                     unsigned int const nBins = 200); // number of bins

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
     * Add continuous profile.
     */
    template <typename TTrack>
    void write(TTrack const& track, Code const PID, double const weight);

    /**
     * Add binned profile.
     */
    void write(GrammageType const Xstart, GrammageType const Xend, Code const PID,
               double const weight);

    /**
     * Returns a summary of this output.
     */
    YAML::Node getSummary() const;

  private:
    ParquetStreamer output_; ///< The parquet streamer for this process.

  public:
    ShowerAxis const& showerAxis_;     ///< conversion between geometry and grammage
    GrammageType dX_;                  ///< binning of profile.
    unsigned int nBins_;               ///< number of profile bins.
    std::vector<ProfileData> profile_; // longitudinal profile

  }; // namespace corsika

} // namespace corsika

#include <corsika/detail/modules/writers/LongitudinalProfileWriterParquet.inl>
