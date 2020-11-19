/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>

#include <fstream>
#include <string>

namespace corsika::track_writer {

  class TrackWriter : public corsika::ContinuousProcess<TrackWriter> {

  public:
    TrackWriter(std::string const& filename)
        : fFilename(filename) {}

    void Init();

    template <typename Particle, typename Track>
    ProcessReturn doContinuous(const Particle&, const Track&);

    template <typename Particle, typename Track>
    LengthType MaxStepLength(const Particle&, const Track&);

  private:
    std::string const fFilename;
    std::ofstream fFile;

    int width = 14;
    int precision = 6;
  };

} // namespace corsika::track_writer

#include <corsika/detail/modules/TrackWriter.inl>
