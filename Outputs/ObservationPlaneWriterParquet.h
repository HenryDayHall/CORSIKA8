/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/output/BaseOutput.h>
#include <corsika/output/ParquetStreamer.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::output {

  class ObservationPlaneWriterParquet : public BaseOutput, private ParquetStreamer {

  public:
    /**
     * Write an observation plane to a directory.
     *
     * @param name    The name of this output.
     */
    ObservationPlaneWriterParquet()
        : ParquetStreamer()
        , event_(0){};

    /**
     * Called at the start of each run.
     */
    void StartOfRun(std::filesystem::path const& directory) final {

      // setup the streamer
      InitStreamer((directory / "particles.parquet").string());

      // build the schema
      AddField("event", parquet::Repetition::REQUIRED, parquet::Type::INT32,
               parquet::ConvertedType::INT_32);
      AddField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
               parquet::ConvertedType::INT_32);
      AddField("energy", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
               parquet::ConvertedType::NONE);
      AddField("radius", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE,
               parquet::ConvertedType::NONE);

      // and build the streamer
      BuildStreamer();
    }

    /**
     * Called at the start of each event/shower.
     */
    void StartOfEvent() final { ++event_; }

    /**
     * Called at the end of each event/shower.
     */
    void EndOfEvent() final {}

    /**
     * Called at the end of each run.
     */
    void EndOfRun() final { CloseStreamer(); }

    /**
     * Get final text outputs for the config file.
     */
    YAML::Node GetOutput() final { return YAML::Node(); }

  protected:
    /**
     * Write a particle to the file.
     */
    void Write(particles::Code const& pid, units::si::HEPEnergyType const& energy,
               units::si::LengthType const& distance) {
      using namespace units::si;

      // write the next row
      (*writer_) << event_ << static_cast<int>(particles::GetPDG(pid)) << energy / 1_eV
                 << distance / 1_m << parquet::EndRow;
    }

  private:
    int event_; ///< The current event number we are processing.

  }; // class ObservationPlaneWriterParquet

} // namespace corsika::output
