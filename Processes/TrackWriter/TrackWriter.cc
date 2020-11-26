/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <iomanip>
#include <limits>

using namespace corsika::setup;
using Particle = Stack::ParticleType;
using Track = Trajectory;

namespace corsika::process::track_writer {

  TrackWriter::TrackWriter(std::string const& filename)
      : fFilename(filename) {

    using namespace std::string_literals;

    fFile.open(fFilename);
    fFile
        << "# PID, E / eV, start coordinates / m, displacement vector to end / m, steplength / m "s
        << '\n';
  }

  template <>
  process::EProcessReturn TrackWriter::DoContinuous(Particle& vP, Track& vT) {
    using namespace units::si;
    auto const start = vT.GetPosition(0).GetCoordinates();
    auto const delta = vT.GetPosition(1).GetCoordinates() - start;
    auto const pdg = static_cast<int>(particles::GetPDG(vP.GetPID()));

    // clang-format off
    fFile << std::setw(7) << pdg
          << std::setw(width) << std::scientific << std::setprecision(precision) << vP.GetEnergy() / 1_eV
          << std::setw(width) << std::scientific << std::setprecision(precision) << start[0] / 1_m 
          << std::setw(width) << std::scientific << std::setprecision(precision) << start[1] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << start[2] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << delta[0] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << delta[1] / 1_m
          << std::setw(width) << std::scientific << std::setprecision(precision) << delta[2] / 1_m 
          << std::setw(width) << std::scientific << std::setprecision(precision) << delta.norm() / 1_m
          << '\n';
    // clang-format on

    return process::EProcessReturn::eOk;
  }

} // namespace corsika::process::track_writer
