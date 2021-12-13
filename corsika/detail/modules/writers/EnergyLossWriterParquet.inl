/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/FindXmax.hpp>

#include <corsika/media/ShowerAxis.hpp>

#include <exception>

namespace corsika {

  inline EnergyLossWriterParquet::EnergyLossWriterParquet() {}

  inline void EnergyLossWriterParquet::startOfLibrary(
      boost::filesystem::path const& directory) {

    // setup the streamer
    output_.initStreamer((directory / "dEdX.parquet").string());

    // enable compression with the default level
    // output_.enableCompression();

    // build the schema
    output_.addField("X", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("total", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);

    // and build the streamer
    output_.buildStreamer();
  }

  inline void EnergyLossWriterParquet::write(unsigned int const showerId,
                                             GrammageType const grammage,
                                             HEPEnergyType const total) {

    double const dX = grammage / 1_g * square(1_cm); // g/cm2

    // and write the data into the column
    *(output_.getWriter()) << showerId << static_cast<float>(dX)
                           << static_cast<float>(total / 1_GeV) << parquet::EndRow;
  }

  inline void EnergyLossWriterParquet::startOfShower(unsigned int const) {}

  inline void EnergyLossWriterParquet::endOfShower(unsigned int const) {}

  inline void EnergyLossWriterParquet::endOfLibrary() { output_.closeStreamer(); }

} // namespace corsika
