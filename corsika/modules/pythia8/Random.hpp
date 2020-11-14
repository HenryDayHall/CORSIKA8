/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <Pythia8/Pythia.h>

#include <corsika/framework/random/RNGManager.hpp>

namespace corsika::pythia8 {

  class Random : public Pythia8::RndmEngine {
    double flat();

  private:
    std::uniform_real_distribution<double> fDist;
    corsika::RNG& fRNG = corsika::RNGManager::getInstance().getRandomStream("pythia");
  };

} // namespace corsika::pythia8

#include <corsika/detail/modules/pythia8/Random.inl>
