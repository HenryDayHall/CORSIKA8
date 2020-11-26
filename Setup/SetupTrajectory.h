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

#include <corsika/process/tracking_line/Tracking.h>
#include <corsika/process/tracking_leapfrog_curved/Tracking.h>
#include <corsika/process/tracking_leapfrog_straight/Tracking.h>

#include <corsika/units/PhysicalUnits.h>

namespace corsika::setup {

  /**
    Note/Warning:     Tracking and Trajectory must fit together !

    tracking_leapfrog_curved::Tracking is the result of the Bachelor
    thesis of Andre Schmidt, KIT. This is a leap-frog algorithm with
    an analytical, precise calculation of volume intersections. This
    algorithm needs a LeapFrogTrajectory.

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

    template <typename TTrack>
    TTrack make_track(const corsika::geometry::Line& line,
                      const corsika::units::si::TimeType tEnd);

    template <>
    inline corsika::geometry::LineTrajectory
    make_track<corsika::geometry::LineTrajectory>(
        const corsika::geometry::Line& line, const corsika::units::si::TimeType tEnd) {
      return corsika::geometry::LineTrajectory(line, tEnd);
    }

    template <>
    inline corsika::geometry::LeapFrogTrajectory
    make_track<corsika::geometry::LeapFrogTrajectory>(
        const corsika::geometry::Line& line, const corsika::units::si::TimeType tEnd) {
      using namespace corsika::units::si;
      typedef corsika::geometry::Vector<magnetic_flux_density_d> MagneticFieldVector;

      auto const k = square(0_m) / (square(1_s) * 1_V);
      return corsika::geometry::LeapFrogTrajectory(
          line.GetR0(), line.GetV0(),
          MagneticFieldVector{line.GetR0().GetCoordinateSystem(), 0_T, 0_T, 0_T}, k,
          tEnd);
    }

  } // namespace testing

} // namespace corsika::setup
