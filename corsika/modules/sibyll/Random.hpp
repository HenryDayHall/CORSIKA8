/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/random/RNGManager.hpp>
#include <random>

namespace sibyll {

  double rndm_interface() {
    static corsika::RNG& rng =
      corsika::RNGManager::GetInstance().GetRandomStream("s_rndm");    
    std::uniform_real_distribution<double> dist;
    return dist(rng);
  }
  
}

