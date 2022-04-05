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

  class ParticleWriterParquet : public BaseOutput {

  public:
    /**
     * Construct an ObservationPlane.
     */
    ParticleWriterParquet();

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

    /**
     * Write a PDG/corsika::Code particle to the file.
     */
    void write(Code const pid, units::si::HEPEnergyType const energy,
               units::si::LengthType const x, units::si::LengthType const y,
               units::si::LengthType const z, units::si::TimeType const time,
               const double weight);

    /**
     * Return collected library-level summary for output.
     */
    YAML::Node getSummary() const final override;

    /**
     * If plane is absorbing particles: return the total energy absorbed.
     */
    HEPEnergyType getEnergyGround() const { return totalEnergy_; }

  private:
    ParquetStreamer output_; ///< The primary output file.
    unsigned int showerId_;  ///< current shower Id

    double countHadrons_ = 0; ///< count hadrons hitting plane
    double countMuons_ = 0;   ///< count muons hitting plane
    double countEM_ = 0;      ///< count EM particles hitting plane.
    double countOthers_ = 0;  ///< count othe types of particles hitting plane

    HEPEnergyType totalEnergy_; ///< energy absorbed in ground.

  }; // class ParticleWriterParquet

} // namespace corsika

#include <corsika/detail/modules/writers/ParticleWriterParquet.inl>
