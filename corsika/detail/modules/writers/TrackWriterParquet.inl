/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  TrackWriterParquet::TrackWriterParquet()
      : output_() {}

  void TrackWriterParquet::startOfLibrary(boost::filesystem::path const& directory) {

    // setup the streamer
    output_.initStreamer((directory / "tracks.parquet").string());

    // build the schema
    output_.addField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                     parquet::ConvertedType::INT_32);
    output_.addField("energy", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("start_x", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("start_y", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("start_z", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("end_x", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("end_y", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("end_z", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);

    // and build the streamer
    output_.buildStreamer();

    setInit(true);
  }

  void TrackWriterParquet::endOfShower() { ++shower_; }

  void TrackWriterParquet::endOfLibrary() { output_.closeStreamer(); }

  void TrackWriterParquet::write(Code const& pid, HEPEnergyType const& energy,
                                 QuantityVector<length_d> const& start,
                                 QuantityVector<length_d> const& end) {

    if (!isInit()) {
      std::runtime_error(
          "TrackWriterParquet not initialized. Either 1) add the corresponding module to "
          "the OutputManager, or 2) declare the module to write no output using "
          "NoOutput.");
    }

    // write the next row - we must write `shower_` first.
    // clang-format off
    *(output_.getWriter())
        << shower_
        << static_cast<int>(get_PDG(pid))
        << static_cast<float>(energy / 1_GeV)
        << static_cast<float>(start[0] / 1_m)
        << static_cast<float>(start[1] / 1_m)
        << static_cast<float>(start[2] / 1_m)
        << static_cast<float>(end[0] / 1_m)
        << static_cast<float>(end[1] / 1_m)
        << static_cast<float>(end[2] / 1_m)
        << parquet::EndRow;
    // clang-format on
  }

} // namespace corsika
