
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _Processes_TrackWriter_h_
#define _Processes_TrackWriter_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <fstream>
#include <limits>
#include <string>

namespace corsika::process::TrackWriter {

  class TrackWriter : public corsika::process::ContinuousProcess<TrackWriter> {

  public:
    TrackWriter(std::string const& filename)
        : fFilename(filename) {}

    void Init();

    template <typename Particle, typename Track, typename Stack>
    corsika::process::EProcessReturn DoContinuous(Particle& p, Track& t, Stack&) {
      using namespace corsika::units::si;
      auto const start = t.GetPosition(0).GetCoordinates();
      auto const delta = t.GetPosition(1).GetCoordinates() - start;
      auto const& name = corsika::particles::GetName(p.GetPID());

      fFile << name << "    " << p.GetEnergy() / 1_eV << ' ' << start[0] / 1_m << ' '
            << start[1] / 1_m << ' ' << start[2] / 1_m << "   " << delta[0] / 1_m << ' '
            << delta[1] / 1_m << ' ' << delta[2] / 1_m << '\n';

      return corsika::process::EProcessReturn::eOk;
    }

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle&, Track&) {
      return corsika::units::si::meter * std::numeric_limits<double>::infinity();
    }

  private:
    std::string const fFilename;
    std::ofstream fFile;
  };

} // namespace corsika::process::TrackWriter

#endif
