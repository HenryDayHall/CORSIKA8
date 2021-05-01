/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/TrackWriter.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <iomanip>
#include <limits>

namespace corsika {

  inline TrackWriter::TrackWriter(std::string const& filename)
      : filename_(filename) {
    using namespace std::string_literals;

    file_.open(filename_);
    file_
        << "# PID, E / eV, start coordinates / m, displacement vector to end / m, steplength / m "s
        << '\n';
  }

  inline TrackWriter::~TrackWriter() { file_.close(); }

  template <typename TParticle, typename TTrack>
  inline ProcessReturn TrackWriter::doContinuous(TParticle const& vP, TTrack const& vT,
                                                 bool const) {

    CORSIKA_LOG_DEBUG("TrackWriter");

    auto const start = vT.getPosition(0).getCoordinates();
    auto const delta = vT.getPosition(1).getCoordinates() - start;
    auto const pdg = static_cast<int>(get_PDG(vP.getPID()));

    // clang-format off
    file_ << std::setw(7) << pdg
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << vP.getEnergy() / 1_eV
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << start[0] / 1_m 
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << start[1] / 1_m
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << start[2] / 1_m
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << delta[0] / 1_m
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << delta[1] / 1_m
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << delta[2] / 1_m 
          << std::setw(width_) << std::scientific << std::setprecision(precision_) << delta.getNorm() / 1_m
          << '\n';
    // clang-format on

    return ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrack>
  inline LengthType TrackWriter::getMaxStepLength(TParticle const&, TTrack const&) {
    return meter * std::numeric_limits<double>::infinity();
  }

} // namespace corsika
