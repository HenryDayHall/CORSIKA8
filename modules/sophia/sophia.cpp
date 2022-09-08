/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <sophia.hpp>

#include <cmath>

double get_sophia_mass2(int& id) { return so_mass1_.am2[std::abs(id) - 1]; }

double rndm_(int&) { return ::sophia::rndm_interface(); }
