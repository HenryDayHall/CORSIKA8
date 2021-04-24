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

  class ObservationPlaneWriterParquet : public BaseOutput {

    ParquetStreamer output_; ///< The primary output file.

  public:
    /**
     * Construct an ObservationPlane.
     *
     * @param name    The name of this output.
     */
    ObservationPlaneWriterParquet();

    /**
     * Called at the start of each library.
     */
    void startOfLibrary(boost::filesystem::path const& directory) final override;

    /**
     * Called at the end of each shower.
     */
    void endOfShower() final override;

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
    void write(Code const& pid, units::si::HEPEnergyType const& energy,
               units::si::LengthType const& x, units::si::LengthType const& y);

  }; // class ObservationPlaneWriterParquet

} // namespace corsika

#include <corsika/detail/modules/writers/ObservationPlaneWriterParquet.inl>
