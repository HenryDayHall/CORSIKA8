/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <urqmd.hpp>

namespace urqmd {
  double ranf_(int&) { return ::urqmd::rndm_interface(); }
}

