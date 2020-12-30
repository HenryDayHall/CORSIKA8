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

  inline VelocityVector LineTrajectory::getVelocity(double const u) const {
    return initialVelocity_ * (1 - u) + finalVelocity_ * u;
  }

  inline TimeType LineTrajectory::getDuration(double const u) const {
    return u * timeStep_;
  }

  inline LengthType LineTrajectory::getLength(double const u) const {
    if (timeLength_ == 0_s) return 0_m;
    if (timeStep_ == std::numeric_limits<TimeType::value_type>::infinity() * 1_s)
      return std::numeric_limits<LengthType::value_type>::infinity() * 1_m;
    return getDistance(u) * timeStep_ / timeLength_;
  }

  ///! set new duration along potentially bend trajectory.
  inline void LineTrajectory::setLength(LengthType const limit) {
    setDuration(line_.getTimeFromArclength(limit));
  }

  ///! set new duration along potentially bend trajectory.
  //   Scale other properties by "limit/timeLength_"
  inline void LineTrajectory::setDuration(TimeType const limit) {
    if (timeStep_ == 0_s) {
      timeLength_ = 0_s;
      setFinalVelocity(getVelocity(0));
      timeStep_ = limit;
    } else {
      // for infinite steps there can't be a difference between
      // curved and straight trajectory, this is fundamentally
      // undefined: assume they are the same (which, i.e. is always correct for a
      // straight line trajectory).
      //
      // Final note: only straight-line trajectories should have
      // infinite steps! Everything else is ill-defined.
      if (timeStep_ == std::numeric_limits<TimeType::value_type>::infinity() * 1_s ||
          timeLength_ == std::numeric_limits<TimeType::value_type>::infinity() * 1_s) {
        timeLength_ = limit;
        timeStep_ = limit;
        // ...and don't touch velocity
      } else {
        const double scale = limit / timeStep_;
        timeLength_ *= scale;
        setFinalVelocity(getVelocity(scale));
        timeStep_ = limit;
      }
    }
  }

  inline LengthType LineTrajectory::getDistance(double const u) const {
    assert(u <= 1);
    assert(u >= 0);
    return line_.getArcLength(0 * second, u * timeLength_);
  }

  inline Line const LeapFrogTrajectory::getLine() const {
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

  ///! duration along potentially bend trajectory
  inline TimeType LeapFrogTrajectory::getDuration(double const u) const {
    return u * timeStep_ *
           (double(getVelocity(u).getNorm() / initialVelocity_.getNorm()) + 1.0) / 2;
  }

  ///! total length along potentially bend trajectory
  inline LengthType LeapFrogTrajectory::getLength(double const u) const {
    return timeStep_ * initialVelocity_.getNorm() * u;
  }

  ///! set new duration along potentially bend trajectory.
  inline void LeapFrogTrajectory::setLength(LengthType const limit) {
    if (initialVelocity_.getNorm() == 0_m / 1_s) setDuration(0_s);
    setDuration(limit / initialVelocity_.getNorm());
  }

  ///! set new duration along potentially bend trajectory.
  //   Scale other properties by "limit/timeLength_"
  inline void LeapFrogTrajectory::setDuration(TimeType const limit) { timeStep_ = limit; }

  /*
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
  */

} // namespace corsika
