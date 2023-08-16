/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
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

  class ObservationVolumeWriterParquet : public BaseOutput {
  private:
    ParquetStreamer output_; ///< The primary output file.
    unsigned int shower_;

  public:
    /**
     * Construct an ObservationVolume.
     *
     * @param name    The name of this output.
     */
    ObservationVolumeWriterParquet();

    /**
     * Called at the start of each library.
     */
    void startOfLibrary(boost::filesystem::path const& directory) final override;

    /**
     * Called at the beginning of each shower.
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

  protected:
    /**
     * Write a particle to the file.
     */
    void write(Code const& pid, HEPEnergyType const& energy, LengthType const& x,
               LengthType const& y, LengthType const& z, double nx, double ny, double nz,
               TimeType const& t);

  }; // class ObservationVolumeWriterParquet

} // namespace corsika

#include <corsika/detail/modules/writers/ObservationVolumeWriterParquet.inl>
