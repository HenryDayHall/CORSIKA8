/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/utl/CorsikaFenv.h>
#include <cfenv>

extern "C" {
#warning No enabling/disabling of floating point exceptions - platform needs better implementation

int feenableexcept(int excepts) { return -1; }

int fedisableexcept(int excepts) { return -1; }
}
