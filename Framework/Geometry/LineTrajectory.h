#ifndef _include_LINETRAJECTORY_H
#define _include_LINETRAJECTORY_H

#include <corsika/geometry/BaseTrajectory.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  class LineTrajectory : public BaseTrajectory {
    using VelocityVec = Vector<corsika::units::SpeedType::dimension_type>;

    Point const r0;
    VelocityVec const v0;

  public:
    LineTrajectory(Point const& pR0, VelocityVec const& pV0)
        : r0(pR0)
        , v0(pV0) {}

    Point GetPosition(corsika::units::TimeType t) const override { return r0 + v0 * t; }
  };

} // namespace corsika::geometry

#endif
