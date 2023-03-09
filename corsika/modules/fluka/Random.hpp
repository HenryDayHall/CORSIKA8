/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <random>

#include <corsika/framework/random/RNGManager.hpp>

namespace fluka {
  /**
   * the random number generator function for FLUKA
   */
  double rndm_interface() {
    static corsika::default_prng_type& rng =
        corsika::RNGManager<>::getInstance().getRandomStream("fluka");
    static std::uniform_real_distribution<double> dist;
    return dist(rng);
  }

} // namespace fluka
