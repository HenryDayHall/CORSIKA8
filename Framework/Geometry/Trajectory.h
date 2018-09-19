#ifndef _include_TRAJECTORY_H
#define _include_TRAJECTORY_H

#include <corsika/geometry/BaseTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  class Trajectory {
    corsika::units::si::TimeType const fTStart, fTEnd;
    BaseTrajectory const& fTrajectory;

  public:
  Trajectory(corsika::units::si::TimeType pTStart, corsika::units::si::TimeType pTEnd,
               BaseTrajectory const& pTrajectory)
        : fTStart(pTStart)
        , fTEnd(pTEnd)
        , fTrajectory(pTrajectory) {}

    Point GetPosition(corsika::units::si::TimeType t) const {
      return fTrajectory.GetPosition(t + fTStart);
    }

    Point GetPosition(double u) const {
      return GetPosition(fTEnd * u + fTStart * (1 - u));
    }
  };

} // namespace corsika::geometry

#endif
