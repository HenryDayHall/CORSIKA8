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

#include <string>

namespace corsika {

  inline BetheBlochPDGWriterParquet::BetheBlochPDGWriterParquet()
      : output_()
      , showerId_(0.) {}

  inline void BetheBlochPDGWriterParquet::startOfLibrary(
      boost::filesystem::path const& directory) {
    // setup the streamer
    output_.initStreamer((directory / "energyloss.parquet").string());

    // enable compression with the default level
    // output_.enableCompression();

    // build the schema
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
    output_.addField("pdg", parquet::Repetition::REQUIRED, parquet::Type::INT32,
                     parquet::ConvertedType::INT_32);
    output_.addField("dE", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);

    // and build the streamer
    output_.buildStreamer();
  }

  inline void BetheBlochPDGWriterParquet::startOfShower(unsigned int const showerId) {
    showerId_ = showerId;
  }

  inline void BetheBlochPDGWriterParquet::endOfShower(unsigned int const showerId) {}

  inline void BetheBlochPDGWriterParquet::endOfLibrary() { output_.closeStreamer(); }

  template <typename TTrack>
  inline void BetheBlochPDGWriterParquet::write(TTrack const& track, Code const pid,
                                                HEPEnergyType dE) {

    auto const start = track.getPosition(0).getCoordinates();
    auto const end = track.getPosition(1).getCoordinates();

    // clang-format off
    *(output_.getWriter()) << showerId_
                           << static_cast<float>(start[0] / 1_m)
                           << static_cast<float>(start[1] / 1_m)
                           << static_cast<float>(start[2] / 1_m)
                           << static_cast<float>(end[0] / 1_m)
                           << static_cast<float>(end[1] / 1_m)
                           << static_cast<float>(end[2] / 1_m)
                           << static_cast<int>(get_PDG(pid))
                           << static_cast<float>(dE / 1_GeV)
                           << parquet::EndRow;
    // clang-format on
  }

  // inline void BetheBlochPDGWriterParquet::write(GrammageType const Xstart,
  //                                               GrammageType const Xend, Code const
  //                                               pid, double const weight) {
  //   double const bstart = Xstart / dX_;
  //   double const bend = Xend / dX_;

  //   if (abs(bstart - floor(bstart + 0.5)) > 1e-2 ||
  //       abs(bend - floor(bend + 0.5)) > 1e-2 || abs(bend - bstart - 1) > 1e-2) {
  //     CORSIKA_LOGGER_ERROR(getLogger(),
  //                          "CONEX and Corsika8 dX grammage binning are not the same! "
  //                          "Xstart={} Xend={} dX={}",
  //                          Xstart / 1_g * square(1_cm), Xend / 1_g * square(1_cm),
  //                          dX_ / 1_g * square(1_cm));
  //     throw std::runtime_error(
  //         "CONEX and Corsika8 dX grammage binning are not the same!");
  //   }

  //   int const bin = int((bend + bstart) / 2);

  //   if (pid == Code::Photon) {
  //     profile_.at(bin)[static_cast<int>(ProfileIndex::Photon)] += weight;
  //   } else if (pid == Code::Positron) {
  //     profile_.at(bin)[static_cast<int>(ProfileIndex::Positron)] += weight;
  //   } else if (pid == Code::Electron) {
  //     profile_.at(bin)[static_cast<int>(ProfileIndex::Electron)] += weight;
  //   } else if (pid == Code::MuPlus) {
  //     profile_.at(bin)[static_cast<int>(ProfileIndex::MuPlus)] += weight;
  //   } else if (pid == Code::MuMinus) {
  //     profile_.at(bin)[static_cast<int>(ProfileIndex::MuMinus)] += weight;
  //   } else if (is_hadron(pid)) {
  //     profile_.at(bin)[static_cast<int>(ProfileIndex::Hadron)] += weight;
  //   }
  //   if (is_charged(pid)) {
  //     profile_[bin][static_cast<int>(ProfileIndex::Charged)] += weight;
  //   }
  // }

  // inline YAML::Node BetheBlochPDGWriterParquet::getSummary() const {
  //   // determined Xmax and dEdXmax from quadratic interpolation

  //   YAML::Node summary;

  //   for (int index = 0; index < static_cast<int>(ProfileIndex::Entries); ++index) {
  //     // first find highest 3-boxcar sum
  //     double maximum = 0;
  //     unsigned int iMaximum = 0;
  //     for (unsigned int i = 0; i < profile_.size() - 3; ++i) {
  //       double value = profile_[i + 0].at(index) + profile_[i + 1].at(index) +
  //                      profile_[i + 2].at(index);
  //       if (value > maximum) {
  //         maximum = value;
  //         iMaximum = i;
  //       }
  //     }

  //     double const dX = dX_ / 1_g * square(1_cm);

  //     // quadratic interpolation of maximum in 3 highest points
  //     auto [Xmax, Nmax] = FindXmax::interpolateProfile(
  //         dX * (0.5 + iMaximum), dX * (1.5 + iMaximum), dX * (2.5 + iMaximum),
  //         profile_[iMaximum + 0].at(index), profile_[iMaximum + 1].at(index),
  //         profile_[iMaximum + 2].at(index));

  //     std::string const name = ProfileIndexNames[index];
  //     summary[name]["Xmax"] = Xmax;
  //     summary[name]["Nmax"] = Nmax;
  //   }
  //   return summary;
  // }

  // inline YAML::Node BetheBlochPDGWriterParquet::getConfig() const {
  //   // determined Xmax and dEdXmax from quadratic interpolation

  //   YAML::Node node;
  //   node["type"] = "BetheBlochPDG";

  //   return node;
  // }

} // namespace corsika
