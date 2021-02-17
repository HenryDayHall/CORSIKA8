/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/random/RNGManager.hpp>
#include <random>

/**
 * \file epos/Random.hpp
 *
 * This file is an integral part of the sibyll interface. It must be
 * linked to the executable linked to sibyll exactly once
 *
 */

namespace epos {

  double rndm_interface() {
    static corsika::default_prng_type& rng =
        corsika::RNGManager::getInstance().getRandomStream("epos");
    std::uniform_real_distribution<double> dist;
    return dist(rng);
  }

} // namespace epos
