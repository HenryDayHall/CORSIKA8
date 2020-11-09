/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Helix.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Trajectory.h>

#include <corsika/units/PhysicalUnits.h>

// #include <variant>

namespace corsika::setup {

  /// definition of Trajectory base class, to be used in tracking and cascades
  typedef corsika::geometry::LineTrajectory Trajectory;

} // namespace corsika::setup
