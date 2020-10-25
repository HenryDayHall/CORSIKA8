/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/output/BaseOutput.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <arrow/io/file.h>
#include <parquet/arrow/schema.h>
#include <parquet/stream_writer.h>

namespace corsika::output {

  class ObservationPlaneWriterParquet : public BaseOutput, private ParquetStreamer {

  public:
    /**
     * Write an observation plane to a directory.
     *
     * @param name    The name of this output.
     */
    ObservationPlaneWriterParquet(std::string const& name)
        : name_(name){};

    /**
     * Called at the start of each run.
     */
    void StartOfRun(std::filesystem::path const& directory) final {

      // (directory / "particles.parquet").string()

      // construct the schema
      auto schema = arrow::schema({arrow::field("pdg", arrow::int64()),
                                   arrow::field("energy", arrow::float64()),
                                   arrow::field("radius", arrow::float64())});

      auto properties = builder.build();
    }

    /**
     * Called at the start of each event/shower.
     */
    void StartOfEvent() final {}

    /**
     * Called at the end of each event/shower.
     */
    void EndOfEvent() final {}

    /**
     * Called at the end of each run.
     */
    void EndOfRun() final {}

  protected:
    void Write(particles::Code const& pid, units::si::HEPEnergyType const& energy,
               units::si::LengthType const& distance) {

      // outputStream_ << static_cast<int>(particles::GetPDG(pid)) << ' '
      //               << particle.GetEnergy() / 1_eV << ' '
      //               << (trajectory.GetPosition(1) - plane_.GetCenter()).norm() / 1_m
      //               << std::endl;
    }

    std::string const name_; ///< The name of this output.

  }; // class ObservationPlaneWriterParquet

} // namespace corsika::output
