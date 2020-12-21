/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>

namespace corsika {

  template <typename TType>
  class Trajectory : public TType {

  public:
    using TType::getArcLength;
    using TType::getPosition;

    Trajectory(TType const& theT, TimeType timeLength)
        : TType(theT)
        , timeLength_(timeLength) {}

    Point getPosition(double const u) const;

    TimeType getDuration() const;

    LengthType getLength() const;

    LengthType getDistance(TimeType const t) const;

    void getLimitEndTo(LengthType const limit);

    auto getNormalizedDirection() const;

  private:
    TimeType timeLength_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Trajectory.inl>
