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

#include <string>
#include <exception>

namespace corsika {

  inline LongitudinalProfileWriterParquet::LongitudinalProfileWriterParquet(
      ShowerAxis const& showerAxis, GrammageType const dX, unsigned int const nBins)
      : output_()
      , showerAxis_(showerAxis)
      , dX_(dX) // profile binning
      , nBins_(nBins) {}

  inline void LongitudinalProfileWriterParquet::startOfLibrary(
      boost::filesystem::path const& directory) {
    // setup the streamer
    output_.initStreamer((directory / "profile.parquet").string());

    // enable compression with the default level
    // output_.enableCompression();

    // build the schema
    output_.addField("X", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("charged", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("hadron", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("muminus", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("muplus", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("photon", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("electron", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);
    output_.addField("positron", parquet::Repetition::REQUIRED, parquet::Type::FLOAT,
                     parquet::ConvertedType::NONE);

    // and build the streamer
    output_.buildStreamer();
  }

  inline void LongitudinalProfileWriterParquet::startOfShower(unsigned int const) {
    // initialize profile
    profile_.clear();
    profile_.resize(nBins_);
  }

  inline void LongitudinalProfileWriterParquet::endOfShower(unsigned int const showerId) {
    int iRow = 0;
    for (ProfileData const& row : profile_) {

      double const dX = dX_ / 1_g * square(1_cm); // g/cm2

      // clang-format off
      *(output_.getWriter()) << showerId << static_cast<float>(iRow * dX)
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::Charged)))
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::Hadron)))
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::MuMinus)))
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::MuPlus)))
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::Photon)))
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::Electron)))
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::Positron)))
                             << parquet::EndRow;
      // clang-format on
      ++iRow;
    }
  }

  inline void LongitudinalProfileWriterParquet::endOfLibrary() {
    output_.closeStreamer();
  }

  template <typename TTrack>
  inline void LongitudinalProfileWriterParquet::write(TTrack const& track, Code const pid,
                                                      double const weight) {
    GrammageType const grammageStart = showerAxis_.getProjectedX(track.getPosition(0));
    GrammageType const grammageEnd = showerAxis_.getProjectedX(track.getPosition(1));

    // Note: particle may go also "upward", thus, grammageEnd<grammageStart
    int const binStart = std::ceil(grammageStart / dX_);
    int const binEnd = std::floor(grammageEnd / dX_);

    for (int bin = binStart; bin <= binEnd; ++bin) {
      if (pid == Code::Photon) {
        profile_.at(bin)[static_cast<int>(ProfileIndex::Photon)] += weight;
      } else if (pid == Code::Positron) {
        profile_.at(bin)[static_cast<int>(ProfileIndex::Positron)] += weight;
      } else if (pid == Code::Electron) {
        profile_.at(bin)[static_cast<int>(ProfileIndex::Electron)] += weight;
      } else if (pid == Code::MuPlus) {
        profile_.at(bin)[static_cast<int>(ProfileIndex::MuPlus)] += weight;
      } else if (pid == Code::MuMinus) {
        profile_.at(bin)[static_cast<int>(ProfileIndex::MuMinus)] += weight;
      } else if (is_hadron(pid)) {
        profile_.at(bin)[static_cast<int>(ProfileIndex::Hadron)] += weight;
      }
      if (is_charged(pid)) {
        profile_[bin][static_cast<int>(ProfileIndex::Charged)] += weight;
      }
    }
  }

  inline void LongitudinalProfileWriterParquet::write(GrammageType const Xstart,
                                                      GrammageType const Xend,
                                                      Code const pid,
                                                      double const weight) {
    double const bstart = Xstart / dX_;
    double const bend = Xend / dX_;

    if (abs(bstart - floor(bstart + 0.5)) > 1e-2 ||
        abs(bend - floor(bend + 0.5)) > 1e-2 || abs(bend - bstart - 1) > 1e-2) {
      CORSIKA_LOGGER_ERROR(getLogger(),
                           "CONEX and Corsika8 dX grammage binning are not the same! "
                           "Xstart={} Xend={} dX={}",
                           Xstart / 1_g * square(1_cm), Xend / 1_g * square(1_cm),
                           dX_ / 1_g * square(1_cm));
      throw std::runtime_error(
          "CONEX and Corsika8 dX grammage binning are not the same!");
    }

    int const bin = int((bend + bstart) / 2);

    if (pid == Code::Photon) {
      profile_.at(bin)[static_cast<int>(ProfileIndex::Photon)] += weight;
    } else if (pid == Code::Positron) {
      profile_.at(bin)[static_cast<int>(ProfileIndex::Positron)] += weight;
    } else if (pid == Code::Electron) {
      profile_.at(bin)[static_cast<int>(ProfileIndex::Electron)] += weight;
    } else if (pid == Code::MuPlus) {
      profile_.at(bin)[static_cast<int>(ProfileIndex::MuPlus)] += weight;
    } else if (pid == Code::MuMinus) {
      profile_.at(bin)[static_cast<int>(ProfileIndex::MuMinus)] += weight;
    } else if (is_hadron(pid)) {
      profile_.at(bin)[static_cast<int>(ProfileIndex::Hadron)] += weight;
    }
    if (is_charged(pid)) {
      profile_[bin][static_cast<int>(ProfileIndex::Charged)] += weight;
    }
  }

  inline YAML::Node LongitudinalProfileWriterParquet::getSummary() const {
    // determined Xmax and dEdXmax from quadratic interpolation

    YAML::Node summary;

    for (int index = 0; index < static_cast<int>(ProfileIndex::Entries); ++index) {
      // first find highest 3-boxcar sum
      double maximum = 0;
      unsigned int iMaximum = 0;
      for (unsigned int i = 0; i < profile_.size() - 3; ++i) {
        double value = profile_[i + 0].at(index) + profile_[i + 1].at(index) +
                       profile_[i + 2].at(index);
        if (value > maximum) {
          maximum = value;
          iMaximum = i;
        }
      }

      double const dX = dX_ / 1_g * square(1_cm);

      // quadratic interpolation of maximum in 3 highest points
      auto [Xmax, Nmax] = FindXmax::interpolateProfile(
          dX * (0.5 + iMaximum), dX * (1.5 + iMaximum), dX * (2.5 + iMaximum),
          profile_[iMaximum + 0].at(index), profile_[iMaximum + 1].at(index),
          profile_[iMaximum + 2].at(index));

      std::string const name = ProfileIndexNames[index];
      summary[name]["Xmax"] = Xmax;
      summary[name]["Nmax"] = Nmax;
    }
    return summary;
  }

} // namespace corsika
