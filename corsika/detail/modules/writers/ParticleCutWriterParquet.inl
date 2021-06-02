/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

namespace corsika {

  inline ParticleCutWriterParquet::ParticleCutWriterParquet()
      : output_() {}

  inline void ParticleCutWriterParquet::startOfLibrary(
      boost::filesystem::path const& directory) {

    // setup the streamer
    output_.initStreamer((directory / "energyloss.parquet").string());

    // build the schema
    output_.addField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                     parquet::ConvertedType::INT_32);
    output_.addField("energy", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("x", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("y", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("z", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);

    // and build the streamer
    output_.buildStreamer();
  }

  inline void ParticleCutWriterParquet::startOfShower(unsigned int const showerId) {
    showerId_ = showerId;
  }

  inline void ParticleCutWriterParquet::endOfShower(unsigned int const) {}

  inline void ParticleCutWriterParquet::endOfLibrary() { output_.closeStreamer(); }

  inline void ParticleCutWriterParquet::write(Point const& point, Code const pid,
                                              HEPEnergyType energy) {

    auto location{point.getCoordinates()};

    // write the next row - we must write `shower_` first.
    // clang-format off
    *(output_.getWriter())
        << showerId_
        << static_cast<int>(get_PDG(pid))
        << static_cast<float>(energy / 1_GeV)
        << static_cast<float>(location[0] / 1_m)
        << static_cast<float>(location[1] / 1_m)
        << static_cast<float>(location[2] / 1_m)
        << parquet::EndRow;
    // clang-format on
  }

} // namespace corsika
