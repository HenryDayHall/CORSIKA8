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
#include <corsika/framework/geometry/PhysicalGeometry.hpp>

namespace corsika {

  inline Line LeapFrogTrajectory::getLine() const {
    auto D = getPosition(1) - getPosition(0);
    auto d = D.getNorm();
    auto v = initialVelocity_;
    if (d > 1_um) { // if trajectory is ultra-short, we do not
                    // re-calculate velocity, just use initial
                    // value. Otherwise, this is numerically unstable
      v = D / d * getVelocity(0).getNorm();
    }
    return Line(getPosition(0), v);
  }

  inline Point LeapFrogTrajectory::getPosition(double const u) const {
    Point position = initialPosition_ + initialVelocity_ * timeStep_ * u / 2;
    VelocityVector velocity =
        initialVelocity_ + initialVelocity_.cross(magneticfield_) * timeStep_ * u * k_;
    return position + velocity * timeStep_ * u / 2;
  }

  inline VelocityVector LeapFrogTrajectory::getVelocity(double const u) const {
    return initialVelocity_ + initialVelocity_.cross(magneticfield_) * timeStep_ * u * k_;
  }

  inline DirectionVector LeapFrogTrajectory::getDirection(double const u) const {
    return getVelocity(u).normalized();
  }

  inline TimeType LeapFrogTrajectory::getDuration(double const u) const {
    return u * timeStep_ *
           (double(getVelocity(u).getNorm() / initialVelocity_.getNorm()) + 1.0) / 2;
  }

  inline LengthType LeapFrogTrajectory::getLength(double const u) const {
    return timeStep_ * initialVelocity_.getNorm() * u;
  }

  inline void LeapFrogTrajectory::setLength(LengthType const limit) {
    if (initialVelocity_.getNorm() == 0_m / 1_s) setDuration(0_s);
    setDuration(limit / initialVelocity_.getNorm());
  }

  inline void LeapFrogTrajectory::setDuration(TimeType const limit) { timeStep_ = limit; }

} // namespace corsika
