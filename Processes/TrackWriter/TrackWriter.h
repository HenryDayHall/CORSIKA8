/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _Processes_track_writer_TrackWriter_h_
#define _Processes_track_writer_TrackWriter_h_

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
  };

} // namespace corsika::process::track_writer

#endif
