#ifndef _include_BASETRAJECTORY_H
#define _include_BASETRAJECTORY_H

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <string>

namespace corsika::geometry {

  /*!
   * Base class for trajectories.
   */
  class BaseTrajectory {

  public:
    //!< t for \f$ t = 0 \f$, the starting Point shall be returned.
    virtual Point GetPosition(corsika::units::si::TimeType t) const = 0;
  };

} // namespace corsika::geometry

#endif
