/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/pythia/Random.h>

namespace corsika::process::pythia {

  double Random::flat() { return fDist(fRNG); }

} // namespace corsika::process::pythia
