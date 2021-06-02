/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/Logging.hpp>

namespace corsika {

  inline ParticleWriterParquet::ParticleWriterParquet()
      : output_()
      , showerId_(0)
      , totalEnergy_(0_eV) {}

  inline void ParticleWriterParquet::startOfLibrary(
      boost::filesystem::path const& directory) {

    // setup the streamer
    output_.initStreamer((directory / "particles.parquet").string());

    // enable compression with the default level
    // output_.enableCompression();

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
    output_.addField("weight", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);

    // and build the streamer
    output_.buildStreamer();

    showerId_ = 0;
    totalEnergy_ = 0_eV;
    countHadrons_ = 0;
    countOthers_ = 0;
    countEM_ = 0;
    countMuons_ = 0;
  }

  inline void ParticleWriterParquet::startOfShower(unsigned int const showerId) {
    showerId_ = showerId;
  }

  inline void ParticleWriterParquet::endOfShower(unsigned int const) {}

  inline void ParticleWriterParquet::endOfLibrary() { output_.closeStreamer(); }

  inline void ParticleWriterParquet::write(Code const& pid, HEPEnergyType const& energy,
                                           LengthType const& x, LengthType const& y,
                                           LengthType const& z,
                                           double const weight) {

    // write the next row - we must write `shower_` first.
    *(output_.getWriter()) << showerId_ << static_cast<int>(get_PDG(pid))
                           << static_cast<float>(energy / 1_GeV)
                           << static_cast<float>(x / 1_m) << static_cast<float>(y / 1_m)
                           << static_cast<float>(z / 1_m)
                           << static_cast<float>(weight) << parquet::EndRow;

    totalEnergy_ += energy;

    if (is_hadron(pid)) {
      ++countHadrons_;
    } else if (is_muon(pid)) {
      ++countMuons_;
    } else if (is_em(pid)) {
      ++countEM_;
    } else {
      ++countOthers_;
    }
  }

  inline void ParticleWriterParquet::write(unsigned int const A, unsigned int const Z,
                                           HEPEnergyType const& energy,
                                           LengthType const& x, LengthType const& y,
                                           LengthType const& z,
                                           double const weight) {
    // write the next row - we must write `shower_` first.
    *(output_.getWriter()) << showerId_ << static_cast<int>(get_PDG(A, Z))
                           << static_cast<float>(energy / 1_GeV)
                           << static_cast<float>(x / 1_m) << static_cast<float>(y / 1_m)
                           << static_cast<float>(z / 1_m)
                           << static_cast<float>(weight) << parquet::EndRow;
    totalEnergy_ += energy;

    ++countHadrons_;
  }

  /**
   * Return collected library-level summary for output.
   */
  YAML::Node ParticleWriterParquet::getSummary() const {
    YAML::Node summary;
    summary["hadrons"] = countHadrons_;
    summary["muons"] = countMuons_;
    summary["em"] = countEM_;
    summary["others"] = countOthers_;
    return summary;
  }

} // namespace corsika
