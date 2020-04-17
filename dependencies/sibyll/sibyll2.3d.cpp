/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <sibyll2.3d.hpp>

#include <cmath>

int get_nwounded() { return s_chist_.nwd; }
double get_sibyll_mass2(int& id) { return s_mass1_.am2[std::abs(id) - 1]; }

double s_rndm_(int&) {
  return sibyll::rndm_interface();
}

