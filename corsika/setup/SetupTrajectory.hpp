n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Helix.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/units/PhysicalUnits.h>

namespace corsika::setup {

  /// definition of Trajectory base class, to be used in tracking and cascades
  typedef corsika::Trajectory<corsika::Line> Trajectory;

  /*
  typedef std::variant<std::monostate, corsika::Trajectory<Line>,
                       corsika::Trajectory<Helix>>
                       Trajectory;

    tracking_leapfrog_straight::Tracking is a more simple and direct
    leap-frog implementation. The two halve steps are coded explicitly
    as two straight segments. Intersections with other volumes are
    calculate only on the straight segments. This algorithm is based
    on LineTrajectory.

    tracking_line::Tracking is a pure straight tracker. It is based on
    LineTrajectory.
   */  
  typedef corsika::process::tracking_leapfrog_curved::Tracking Tracking;
  //typedef corsika::process::tracking_leapfrog_straight::Tracking Tracking;
  //typedef corsika::process::tracking_line::Tracking Tracking;

  /// definition of Trajectory base class, to be used in tracking and cascades
  //typedef corsika::geometry::LineTrajectory Trajectory;
  typedef corsika::geometry::LeapFrogTrajectory Trajectory;

  /**

     The following section is for unit testing only. Eventually it should
     be moved to "tests".
    
    
   */
  
  namespace testing {

  /// helper visitor to modify Particle by moving along Trajectory
  class GetDuration {
  public:
    TimeType operator()(std::monostate const&) {
      return 0 * second;
    }
    template <typename T>
    TimeType operator()(T const& trajectory) {
      return trajectory.GetDuration();
    }
  };
  */
} // namespace corsika::setup
