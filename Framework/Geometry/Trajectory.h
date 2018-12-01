
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_TRAJECTORY_H
#define _include_TRAJECTORY_H

//#include <corsika/geometry/BaseTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

  template <typename T>
  class Trajectory : public T { // BaseTrajectory {

    // T fTraj;
    corsika::units::si::TimeType fTStart, fTEnd;

  public:
    using T::GetPosition;
    using T::GetDistanceBetween;

    Trajectory(T const& theT, corsika::units::si::TimeType pTStart,
               corsika::units::si::TimeType pTEnd)
        : T(theT)
        , fTStart(pTStart)
        , fTEnd(pTEnd) {}
    //: BaseTrajectory(pTStart, pTEnd)
    //  , fTraj(theT) {}

    /*Point GetPosition(corsika::units::si::TimeType t) const {
      return fTraj.GetPosition(t + fTStart);
      }*/

    Point GetPosition(double u) const {
      return T::GetPosition(fTEnd * u + fTStart * (1 - u));
    }

    /*
    LengthType GetDistance(corsika::units::si::TimeType t1,
                           corsika::units::si::TimeType t2) const {
      return fTraj.DistanceBetween(t1, t2);
    }
    */
  };

} // namespace corsika::geometry

#endif
