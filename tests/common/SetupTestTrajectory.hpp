#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Helix.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

#include <corsika/modules/TrackingLine.hpp>
//#include <corsika/modules/TrackingCurved.hpp> // simple leap-frog implementation
//#include <corsika/modules/TrackingLeapFrog.hpp> // more complete leap-frog
// implementation

namespace corsika::setup::testing {

  template <typename TTrack>
  TTrack make_track(Line const& line, TimeType const tEnd);

  template <>
  inline LineTrajectory make_track<LineTrajectory>(Line const& line,
                                                   TimeType const tEnd) {
    return LineTrajectory(line, tEnd);
  }

  /*
    template <>
    inline LeapFrogTrajectory make_track<LeapFrogTrajectory>(Line const& line,
                                                             TimeType const tEnd) {

      auto const k = square(0_m) / (square(1_s) * 1_V);
      return LeapFrogTrajectory(
          line.getStartPoint(), line.getVelocity(),
          MagneticFieldVector{line.getStartPoint().getCoordinateSystem(), 0_T, 0_T, 0_T},
          k, tEnd);
    }
  */
} // namespace corsika::setup::testing
