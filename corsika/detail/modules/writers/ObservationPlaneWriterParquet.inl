/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  inline ObservationPlaneWriterParquet::ObservationPlaneWriterParquet()
      : output_() {}

  inline void ObservationPlaneWriterParquet::startOfLibrary(
      boost::filesystem::path const& directory) {

    // setup the streamer
    output_.initStreamer((directory / "particles.parquet").string());

    // enable compression with the default level
    output_.enableCompression();

    // build the schema
    output_.addField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                     parquet::ConvertedType::INT_32);
    output_.addField("energy", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("x", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("y", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);

    // and build the streamer
    output_.buildStreamer();
  }

  inline void ObservationPlaneWriterParquet::endOfShower() { ++shower_; }

  inline void ObservationPlaneWriterParquet::endOfLibrary() { output_.closeStreamer(); }

  inline void ObservationPlaneWriterParquet::write(Code const& pid,
                                                   HEPEnergyType const& energy,
                                                   LengthType const& x,
                                                   LengthType const& y) {
    // write the next row - we must write `shower_` first.
    *(output_.getWriter()) << shower_ << static_cast<int>(get_PDG(pid))
                           << static_cast<float>(energy / 1_GeV)
                           << static_cast<float>(x / 1_m) << static_cast<float>(y / 1_m)
                           << parquet::EndRow;
  }

} // namespace corsika
