
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/track_writer/TrackWriter.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <limits>

using namespace corsika::setup;
using Particle = Stack::ParticleType;
using Track = Trajectory;

namespace corsika::process::TrackWriter {

  void TrackWriter::Init() {
    using namespace std::string_literals;

    fFile.open(fFilename);
    fFile << "# PID, E / eV, start coordinates / m, displacement vector to end / m "s
          << '\n';
  }

  template <>
  process::EProcessReturn TrackWriter::DoContinuous(Particle& p, Track& t, Stack&) {
    using namespace units::si;
    auto const start = t.GetPosition(0).GetCoordinates();
    auto const delta = t.GetPosition(1).GetCoordinates() - start;
    auto const& name = particles::GetName(p.GetPID());

    fFile << name << "    " << p.GetEnergy() / 1_eV << ' ' << start[0] / 1_m << ' '
          << start[1] / 1_m << ' ' << start[2] / 1_m << "   " << delta[0] / 1_m << ' '
          << delta[1] / 1_m << ' ' << delta[2] / 1_m << '\n';

    return process::EProcessReturn::eOk;
  }

  template <>
  units::si::LengthType TrackWriter::MaxStepLength(Particle&, Track&) {
    return units::si::meter * std::numeric_limits<double>::infinity();
  }

} // namespace corsika::process::TrackWriter
