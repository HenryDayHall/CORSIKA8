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

  inline EnergyLossWriterParquet::EnergyLossWriterParquet(ShowerAxis const& showerAxis,
                                                          GrammageType const dX,
                                                          unsigned int const nBins,
                                                          GrammageType const dX_threshold)
      : showerAxis_(showerAxis)
      , dX_(dX) // profile binning
      , nBins_(nBins)
      , dX_threshold_(dX_threshold) {}

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

  inline void EnergyLossWriterParquet::startOfShower(unsigned int const) {

    // initialize profile
    profile_.clear();
    profile_.resize(nBins_);
  }

  inline void EnergyLossWriterParquet::endOfShower(unsigned int const showerId) {

    int iRow = 0;
    for (Profile const& row : profile_) {

      double const dX = dX_ / 1_g * square(1_cm); // g/cm2

      // clang-format off
      *(output_.getWriter()) << showerId << static_cast<float>(iRow * dX)
                             << static_cast<float>(row.at(static_cast<int>(ProfileIndex::Total)) / 1_GeV / dX)
                             << parquet::EndRow;
      // clang-format on
      iRow++;
    }
  }

  inline void EnergyLossWriterParquet::endOfLibrary() { output_.closeStreamer(); }

  template <typename TTrack>
  inline void EnergyLossWriterParquet::write(TTrack const& track, Code const PID,
                                             HEPEnergyType const dE) {

    GrammageType grammageStart = showerAxis_.getProjectedX(track.getPosition(0));
    GrammageType grammageEnd = showerAxis_.getProjectedX(track.getPosition(1));

    if (grammageStart > grammageEnd) { // particle going upstream
      std::swap(grammageStart, grammageEnd);
    }

    GrammageType const deltaX = grammageEnd - grammageStart;

    if (deltaX < dX_threshold_) {
      write(track.getPosition(0), PID, dE);
      return;
    }

    // only register the range that is covered by the profile
    int const maxBin = int(profile_.size() - 1);
    int binStart = grammageStart / dX_;
    if (binStart < 0) binStart = 0;
    if (binStart > maxBin) binStart = maxBin;
    int binEnd = grammageEnd / dX_;
    if (binEnd < 0) binEnd = 0;
    if (binEnd > maxBin) binEnd = maxBin;

    CORSIKA_LOGGER_TRACE(getLogger(), "energy deposit of dE={} GeV between {} and {}",
                         dE / 1_GeV, grammageStart / 1_g * square(1_cm),
                         grammageEnd / 1_g * square(1_cm));

    auto energyCount = HEPEnergyType::zero();

    auto const factor = dE / deltaX;
    auto fill = [&](int const bin, GrammageType const weight) {
      auto const increment = factor * weight;
      profile_[bin][static_cast<int>(ProfileIndex::Total)] += increment;
      energyCount += increment;

      CORSIKA_LOGGER_TRACE(getLogger(), "filling bin={} with weight {} : dE={} GeV ", bin,
                           weight, increment / 1_GeV);
    };

    // fill longitudinal profile
    if (binStart == binEnd) {
      fill(binStart, deltaX);
    } else {
      fill(binStart, ((1 + binStart) * dX_ - grammageStart));
      fill(binEnd, (grammageEnd - binEnd * dX_));
      for (int bin = binStart + 1; bin < binEnd; ++bin) { fill(bin, dX_); }
    }

    CORSIKA_LOGGER_TRACE(getLogger(), "total energy added to histogram: {} GeV ",
                         energyCount / 1_GeV);
  }

  inline void EnergyLossWriterParquet::write(Point const& point, Code const PID,
                                             HEPEnergyType const dE) {
    GrammageType grammage = showerAxis_.getProjectedX(point);
    int const maxBin = int(profile_.size() - 1);
    int bin = grammage / dX_;
    if (bin < 0) bin = 0;
    if (bin > maxBin) bin = maxBin;

    CORSIKA_LOGGER_TRACE(getLogger(), "add local energy loss bin={} dE={} GeV ", bin,
                         dE / 1_GeV);

    profile_[bin][static_cast<int>(ProfileIndex::Total)] += dE;
  }

  inline void EnergyLossWriterParquet::write(GrammageType const Xstart,
                                             GrammageType const Xend,
                                             HEPEnergyType const dE) {
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
    CORSIKA_LOGGER_TRACE(getLogger(), "add binned energy loss {} {} bin={} dE={} GeV ",
                         bstart, bend, bin, dE / 1_GeV);
    profile_[bin][static_cast<int>(ProfileIndex::Total)] += dE;
  }

  inline HEPEnergyType EnergyLossWriterParquet::getTotal() const {
    HEPEnergyType tot = HEPEnergyType::zero();
    for (Profile const& row : profile_)
      tot += row.at(static_cast<int>(ProfileIndex::Total));
    return tot;
  }

  inline YAML::Node EnergyLossWriterParquet::getSummary() const {

    // determined Xmax and dEdXmax from quadratic interpolation
    double maximum = 0;
    unsigned int iMaximum = 0;
    for (unsigned int i = 0; i < profile_.size() - 3; ++i) {
      double value = (profile_[i + 0].at(static_cast<int>(ProfileIndex::Total)) +
                      profile_[i + 1].at(static_cast<int>(ProfileIndex::Total)) +
                      profile_[i + 2].at(static_cast<int>(ProfileIndex::Total))) /
                     1_GeV;
      if (value > maximum) {
        maximum = value;
        iMaximum = i;
      }
    }

    double const dX = dX_ / 1_g * square(1_cm);

    auto [Xmax, dEdXmax] = FindXmax::interpolateProfile(
        dX * (0.5 + iMaximum), dX * (1.5 + iMaximum), dX * (2.5 + iMaximum),
        profile_[iMaximum + 0].at(static_cast<int>(ProfileIndex::Total)) / 1_GeV,
        profile_[iMaximum + 1].at(static_cast<int>(ProfileIndex::Total)) / 1_GeV,
        profile_[iMaximum + 2].at(static_cast<int>(ProfileIndex::Total)) / 1_GeV);

    YAML::Node summary;
    summary["total"] = getTotal() / 1_GeV;
    summary["Xmax"] = Xmax;
    summary["dEdXmax"] = dEdXmax;
    return summary;
  }

} // namespace corsika
