/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/utility/CorsikaFenv.hpp>
#include <cfenv>

extern "C" {
#warning No enabling/disabling of floating point exceptions - platform needs better implementation

inline int feenableexcept(int /*excepts*/) throw() { return -1; }

inline int fedisableexcept(int /*excepts*/) throw() { return -1; }
}
