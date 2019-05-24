/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

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

    BaseTrajectory() = delete;

  public:
    BaseTrajectory(corsika::units::si::TimeType start, corsika::units::si::TimeType end)
        : fTStart(start)
        , fTEnd(end) {}

    //!< for \f$ t = 0 \f$, the starting Point shall be returned.
    virtual Point GetPosition(corsika::units::si::TimeType) const = 0;

    //!< the Point is return from u=0 (start) to u=1 (end)
    virtual Point GetPosition(double u) const = 0;

    /*!
     * returns the length between two points of the trajectory
     * parameterized by \arg t1 and \arg t2. Requires \arg t2 > \arg t1.
     */

    virtual corsika::units::si::TimeType TimeFromArclength(
        corsika::units::si::LengthType) const = 0;

    virtual LengthType ArcLength(corsika::units::si::TimeType t1,
                                 corsika::units::si::TimeType t2) const = 0;

    virtual corsika::units::si::TimeType GetDuration(
        corsika::units::si::TimeType t1, corsika::units::si::TimeType t2) const {
      return t2 - t1;
    }

    virtual Point GetEndpoint() const { return GetPosition(fTEnd); }
    virtual Point GetStartpoint() const { return GetPosition(fTStart); }

  protected:
    corsika::units::si::TimeType const fTStart, fTEnd;
  };

} // namespace corsika::geometry

#endif
