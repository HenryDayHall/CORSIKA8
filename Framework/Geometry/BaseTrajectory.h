#ifndef _include_BASETRAJECTORY_H
#define _include_BASETRAJECTORY_H

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <string>

namespace corsika::geometry {

  /*!
   * Interface / base class for trajectories.
   */
  class BaseTrajectory {

  public:
    //!< for \f$ t = 0 \f$, the starting Point shall be returned.
    virtual Point GetPosition(corsika::units::si::TimeType) const = 0;
    
    /*!
     * returns the arc length between two points of the trajectory
     * parameterized by \arg t1 and \arg t2. Requires \arg t2 > \arg t1.
    
    virtual LengthType DistanceBetween(corsika::units::si::TimeType t1,
                                       corsika::units::si::TimeType t2) const = 0;
  };

} // namespace corsika::geometry

#endif
