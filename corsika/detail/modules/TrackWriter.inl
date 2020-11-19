/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
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

namespace corsika::track_writer {

  void TrackWriter::Init() {
    using namespace std::string_literals;

    fFile.open(fFilename);
    fFile << "# PID, E / eV, start coordinates / m, displacement vector to end / m "s
          << '\n';
  }

  template <typename TParticle, typename TTrack>
  corsika::ProcessReturn TrackWriter::doContinuous(const TParticle& vP,
                                                   const TTrack& vT) {
    auto const start = vT.GetPosition(0).GetCoordinates();
    auto const delta = vT.GetPosition(1).GetCoordinates() - start;
    auto const pdg = static_cast<int>(corsika::get_PDG(vP.GetPID()));

    // clang-format off
    fFile << std::setw(7) << pdg
          << std::setw(width) << std::scientific << std::setprecision(precision) << vP.GetEnergy() / 1_eV
          << std::setw(width) << std::scientific << std::setprecision(precision) << start[0] / 1_m 
          << std::setw(width) << std::scientific << std::setprecision(precision) << start[1] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << start[2] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << delta[0] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << delta[1] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << delta[2] / 1_m << '\n';
    // clang-format on

    return corsika::ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrack>
  LengthType TrackWriter::MaxStepLength(const TParticle&, const TTrack&) {
    return meter * std::numeric_limits<double>::infinity();
  }

} // namespace corsika::track_writer
