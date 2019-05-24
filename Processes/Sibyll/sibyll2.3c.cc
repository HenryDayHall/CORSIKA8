/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/sibyll/sibyll2.3c.h>

#include <corsika/random/RNGManager.h>
#include <random>

int get_nwounded() { return s_chist_.nwd; }
double get_sibyll_mass2(int& id) { return s_mass1_.am2[id]; }

double s_rndm_(int&) {
  static corsika::random::RNG& rng =
      corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");

  std::uniform_real_distribution<double> dist;
  return dist(rng);
}
