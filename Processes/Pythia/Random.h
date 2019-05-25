/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_process_pythia_random_h_
#define _include_corsika_process_pythia_random_h_

#include <Pythia8/Pythia.h>
#include <corsika/random/RNGManager.h>

namespace corsika::process {

  namespace pythia {

    class Random : public Pythia8::RndmEngine {
      double flat();

    private:
      std::uniform_real_distribution<double> fDist;
      corsika::random::RNG& fRNG =
          corsika::random::RNGManager::GetInstance().GetRandomStream("pythia");
    };

  } // namespace pythia
} // namespace corsika::process

#endif
