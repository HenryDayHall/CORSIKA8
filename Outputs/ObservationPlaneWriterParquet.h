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

  class ObservationPlaneWriterParquet : public BaseOutput {

  public:
    /**
     * Write an observation plane to a directory.
     *
     * @param name    The name of this output.
     */
    ObservationPlaneWriterParquet(std::string const& name)
        : name_(name)
        , event_(0){};

    /**
     * Called at the start of each run.
     */
    void StartOfRun(std::filesystem::path const& directory) final {

      // setup the streamer
      streamer_.Init((directory / "particles.parquet").string());

      // build the schema
      streamer_.AddField("event", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                         parquet::ConvertedType::INT_32);
      streamer_.AddField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                         parquet::ConvertedType::INT_32);
      streamer_.AddField("energy", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                         parquet::ConvertedType::NONE);
      streamer_.AddField("radius", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                         parquet::ConvertedType::NONE);

      // and build the streamer
      streamer_.Build();
    }

    /**
     * Called at the start of each event/shower.
     */
      void StartOfEvent() final { ++event_;
      }

    /**
     * Called at the end of each event/shower.
     */
      void EndOfEvent() final {
      }

    /**
     * Called at the end of each run.
     */
      void EndOfRun() final { streamer_.Close();
      }

  protected:
    /**
     * Write a particle to the file.
     */
    void Write(particles::Code const& pid, units::si::HEPEnergyType const& energy,
               units::si::LengthType const& distance) {

      using namespace units::si;

      // write the next row
      writer_ << event_ << static_cast<int>(particles::GetPDG(pid)) << energy / 1_eV
              << distance / 1_m << parquet::EndRow;

    }


    std::string const name_; ///< The name of this output.

  private:
    int event_;                    ///< The current event number we are processing.
    ParquetStreamer streamer_;     ///< A parquet stream writer helper
    parquet::StreamWriter writer_; ///< The writer for this file.

  }; // class ObservationPlaneWriterParquet

} // namespace corsika::output
