/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  ObservationPlaneWriterParquet::ObservationPlaneWriterParquet()
      : ParquetStreamer(){}

  void ObservationPlaneWriterParquet::startOfLibrary(
      std::filesystem::path const& directory) {

    // setup the streamer
    initStreamer((directory / "particles.parquet").string());

    // build the schema
    addField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
             parquet::ConvertedType::INT_32);
    addField("energy", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
             parquet::ConvertedType::NONE);
    addField("x", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
             parquet::ConvertedType::NONE);
    addField("y", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
             parquet::ConvertedType::NONE);

    // and build the streamer
    buildStreamer();
  }

  void ObservationPlaneWriterParquet::endOfShower() { ++shower_; }

  void ObservationPlaneWriterParquet::endOfLibrary() { closeStreamer(); }

  void ObservationPlaneWriterParquet::write(Code const& pid,
                                            units::si::HEPEnergyType const& energy,
                                            units::si::LengthType const& x,
                                            units::si::LengthType const& y) {
    using namespace units::si;

    // write the next row - we must write `shower_` first.
    (*writer_) << shower_
               << static_cast<int>(get_PDG(pid))
               << static_cast<float>(energy / 1_eV)
               << static_cast<float>(x / 1_m)
               << static_cast<float>(y / 1_m)
               << parquet::EndRow;
  }

} // namespace corsika
