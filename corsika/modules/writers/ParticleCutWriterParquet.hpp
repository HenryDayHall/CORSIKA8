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

namespace corsika {

  class ParticleCutWriterParquet : public BaseOutput {

  public:
    /**
     * Construct a new writer.
     */
    ParticleCutWriterParquet();

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
     * Write a particle cut.
     */
    void write(Point const& point, Code const PID, HEPEnergyType const energy);

  private:
    ParquetStreamer output_; ///< The parquet streamer for this process.
    unsigned int showerId_{0};
  }; // namespace corsika

} // namespace corsika

#include <corsika/detail/modules/writers/ParticleCutWriterParquet.inl>
