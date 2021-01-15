/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/random/RNGManager.hpp>
#include <random>

namespace qgsjetII {

  double rndm_interface() {
    static corsika::default_prng_type& rng =
        corsika::RNGManager::getInstance().getRandomStream("qgsjet");
    std::uniform_real_distribution<double> dist;
    return dist(rng);
  }

} // namespace qgsjetII
