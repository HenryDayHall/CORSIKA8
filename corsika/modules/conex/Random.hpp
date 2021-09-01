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
 * \file conex/Random.hpp
 *
 * This file is an integral part of the epos interface. It must be
 * linked to the executable linked to epos exactly once
 *
 * Note, that the fortran random numbe interface functions are all
 * defined in the epos corsika 8 interface:
 *
 * ranfst, ranfgt, rmmaqd, ranfini, ranfcv, rmmard, rangen, drangen
 *
 * All of them use the epos_random_interface registered as "epos" stream.
 *
 * Thus, the fortran part of CONEX will use the "epos" CORSIKA 8 random stream,
 * only the CONEX c++ part will use the "conex" random stream.
 *
 * Since EPOS and CONEX use the same fortran symbols for access to
 * random numbers this can only be changed by renaming inside the
 * fortran part.
 */

namespace conex {

  float rndm_interface() {
    static corsika::default_prng_type& rng =
        corsika::RNGManager<>::getInstance().getRandomStream("conex");
    std::uniform_real_distribution<float> dist;
    return dist(rng);
  }

  double double_rndm_interface() {
    static corsika::default_prng_type& rng =
        corsika::RNGManager<>::getInstance().getRandomStream("conex");
    std::uniform_real_distribution<double> dist;
    return dist(rng);
  }

} // namespace conex
