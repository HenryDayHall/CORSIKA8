/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/random/RNGManager.hpp>
#include <random>

/**
 * \file sophia/Random.hpp
 *
 * This file is an integral part of the sophia interface. It must be
 * linked to the executable linked to sophia exactly once
 *
 */

namespace sophia {

  double rndm_interface() {
    static corsika::default_prng_type& rng =
        corsika::RNGManager<>::getInstance().getRandomStream("sophia");
    std::uniform_real_distribution<double> dist;
    return dist(rng);
  }

} // namespace sophia
