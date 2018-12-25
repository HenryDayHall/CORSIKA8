/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/track_writer/TrackWriter.h>
#include <string>

void corsika::process::TrackWriter::TrackWriter::Init() {
  using namespace std::string_literals;

  fFile.open(fFilename);
  fFile << "# PID, E / eV, start coordinates / m, displacement vector to end / m "s
        << '\n';
}
