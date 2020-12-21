/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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

  template <typename TType>
  Point Trajectory<TType>::getPosition(double const u) const {
    return TType::getPosition(timeLength_ * u);
  }

  template <typename TType>
  TimeType Trajectory<TType>::getDuration() const {
    return timeLength_;
  }

  template <typename TType>
  LengthType Trajectory<TType>::getLength() const {
    return getDistance(timeLength_);
  }

  template <typename TType>
  LengthType Trajectory<TType>::getDistance(TimeType const t) const {
    assert(t <= timeLength_);
    assert(t >= 0 * second);
    return TType::getArcLength(0 * second, t);
  }

  template <typename TType>
  void Trajectory<TType>::getLimitEndTo(LengthType const limit) {
    timeLength_ = TType::getTimeFromArclength(limit);
  }

  template <typename TType>
  auto Trajectory<TType>::getNormalizedDirection() const {
    static_assert(std::is_same_v<TType, corsika::Line>);
    return TType::getVelocity().normalized();
  }

} // namespace corsika
