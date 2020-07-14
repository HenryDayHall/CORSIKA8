/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/ContinuousProcess.h>
#include <corsika/units/PhysicalUnits.h>

#include <fstream>
#include <string>

namespace corsika::process::track_writer {

  class TrackWriter : public corsika::process::ContinuousProcess<TrackWriter> {

  public:
    TrackWriter(std::string const& filename)
        : fFilename(filename) {}

    void Init();

    template <typename Particle, typename Track>
    corsika::process::EProcessReturn DoContinuous(Particle&, Track&);

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle&, Track&);

  private:
    std::string const fFilename;
    std::ofstream fFile;

    int width = 14;
    int precision = 6;
  };

} // namespace corsika::process::track_writer
