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

  class ObservationPlaneWriterParquet : public BaseOutput, private ParquetStreamer {

  public:
    /**
     * Write an observation plane to a directory.
     *
     * @param name    The name of this output.
     */
    ObservationPlaneWriterParquet()
        : ParquetStreamer(){};

    /**
     * Called at the start of each run.
     */
    void startOfRun(std::filesystem::path const& directory) final override {

      // setup the streamer
      initStreamer((directory / "particles.parquet").string());

      // build the schema
      addField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
               parquet::ConvertedType::INT_32);
      addField("energy", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
               parquet::ConvertedType::NONE);
      addField("x", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
               parquet::ConvertedType::NONE);
      addField("y", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
               parquet::ConvertedType::NONE);
      addField("radius", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
               parquet::ConvertedType::NONE);

      // and build the streamer
      buildStreamer();
    }

    /**
     * Called at the end of each event/shower.
     */
    void endOfEvent() final override { ++event_; }

    /**
     * Called at the end of each run.
     *
     * This must also increment the run number since we override
     * the default behaviour of BaseOutput.
     */
    void endOfRun() final override {
      closeStreamer();
      ++run_;
    }

  protected:
    /**
     * Write a particle to the file.
     */
    void write(Code const& pid, units::si::HEPEnergyType const& energy,
               units::si::LengthType const& x, units::si::LengthType const& y,
               units::si::LengthType const& radius) {
      using namespace units::si;

      // write the next row
      // NOTE: we must write run_ and then event_ first
      (*writer_) << run_ << event_ << static_cast<int>(get_PDG(pid)) << energy / 1_eV
                 << x / 1_m << y / 1_m << radius / 1_m << parquet::EndRow;
    }

  }; // class ObservationPlaneWriterParquet

} // namespace corsika
