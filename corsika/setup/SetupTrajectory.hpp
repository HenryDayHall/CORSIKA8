n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

namespace corsika::setup {

  /// definition of Trajectory base class, to be used in tracking and cascades
  typedef corsika::Trajectory<corsika::Line> Trajectory;

} // namespace corsika::setup
