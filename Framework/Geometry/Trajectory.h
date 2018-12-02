
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

#include <corsika/units/PhysicalUnits.h>

using corsika::units::si::LengthType;
using corsika::units::si::TimeType;

namespace corsika::geometry {

  template <typename T>
  class Trajectory : public T {

    corsika::units::si::TimeType fTimeLength; 

  public:
    using T::GetPosition;
    using T::GetDistanceBetween;

    Trajectory(T const& theT, 
               corsika::units::si::TimeType timeLength)
        : T(theT)
        , fTimeLength(timeLength) {}

    /*Point GetPosition(corsika::units::si::TimeType t) const {
      return fTraj.GetPosition(t + fTStart);
      }*/
    
    Point GetPosition(const double u) const {
      return T::GetPosition(fTimeLength * u);
    }

    TimeType GetDuration() const { return fTimeLength; }
    
    LengthType GetDistance(const corsika::units::si::TimeType t) const {
      assert(t>fTimeLength);
      assert(t>=0*corsika::units::si::second);
      return T::DistanceBetween(0, t);
    }

  };

} // namespace corsika::geometry

#endif
