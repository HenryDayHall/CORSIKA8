/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

    template <typename T>
    Point Trajectory<T>::GetPosition(double u) const
    {
    	return T::GetPosition(fTimeLength * u);
    }

    template <typename T>
    corsika::units::si::TimeType Trajectory<T>::GetDuration() const
    {
    	return fTimeLength;
    }

    template <typename T>
    corsika::units::si::LengthType Trajectory<T>::GetLength() const
    {
    	return GetDistance(fTimeLength);
    }

    template <typename T>
    corsika::units::si::LengthType Trajectory<T>::GetDistance(corsika::units::si::TimeType t) const
    {
      assert(t <= fTimeLength);
      assert(t >= 0 * corsika::units::si::second);
      return T::ArcLength(0 * corsika::units::si::second, t);
    }

    template <typename T>
    void Trajectory<T>::LimitEndTo(corsika::units::si::LengthType limit)
    {
      fTimeLength = T::TimeFromArclength(limit);
    }

    template <typename T>
    auto Trajectory<T>::NormalizedDirection() const
    {
      static_assert(std::is_same_v<T, corsika::Line>);
      return T::GetV0().normalized();
    }


} // namespace corsika


