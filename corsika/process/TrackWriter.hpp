/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/sequence/ContinuousProcess.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <fstream>
#include <string>

namespace corsika::track_writer {

  class TrackWriter : public corsika::ContinuousProcess<TrackWriter> {

  public:
    TrackWriter(std::string const& filename);

    template <typename Particle, typename Track>
    corsika::EProcessReturn DoContinuous(Particle&, Track&);

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle&, Track&) {
      return units::si::meter * std::numeric_limits<double>::infinity();
    }

  private:
    std::string const fFilename;
    std::ofstream fFile;

    int width = 14;
    int precision = 6;
  };

} // namespace corsika::track_writer

