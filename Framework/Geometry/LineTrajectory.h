#ifndef _include_LINETRAJECTORY_H
#define _include_LINETRAJECTORY_H

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  class LineTrajectory // TODO: inherit from Trajectory
  {
    using SpeedVec = Vector<corsika::units::SpeedType::dimension_type>;

    Point const r0;
    SpeedVec const v0;

  public:
    LineTrajectory(Point const& pR0, SpeedVec const& pV0)
        : r0(r0)
        , v0(pV0) {}

    auto GetPosition(corsika::units::TimeType t) const { return r0 + v0 * t; }
  };

} // namespace corsika::geometry

#endif
