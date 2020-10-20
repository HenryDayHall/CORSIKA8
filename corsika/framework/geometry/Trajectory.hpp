/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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

  /**
   * \class LineTrajectory
   *
   * A Trajectory is a description of a momvement of an object in
   * three-dimensional space that describes the trajectory (connection
   * between two Points in space), as well as the direction of motion
   * at any given point.
   *
   * A Trajectory has a start `0` and an end `1`, where
   * e.g. GetPosition(0) returns the start point and GetDirection(1)
   * the direction of motion at the end. Values outside 0...1 are not
   * defined.
   *
   * A Trajectory has a length in [m], GetLength, a duration in [s], GetDuration.
   *
   * Note: so far it is assumed that the speed (d|vec{r}|/dt) between
   * start and end does not change and is constant for the entire
   * Trajectory.
   *
   **/

  class LineTrajectory {

    using VelocityVec = Vector<corsika::units::si::SpeedType::dimension_type>;

  public:
    LineTrajectory() = delete;
    LineTrajectory(const LineTrajectory&) = default;
    LineTrajectory(LineTrajectory&&) = default;
    LineTrajectory& operator=(const LineTrajectory&) = delete;

    /**
     * \param theLine The geometric \sa Line object that represents a straight-line
     * connection 
     *
     * \param timeLength The time duration to traverse the straight trajectory
     * in units of \sa TimeType
     */
    LineTrajectory(Line const& theLine, corsika::units::si::TimeType timeLength)
        : line_(theLine)
        , timeLength_(timeLength)
        , timeStep_(timeLength)
        , initialVelocity_(theLine.GetVelocity(corsika::units::si::TimeType::zero()))
        , finalVelocity_(theLine.GetVelocity(timeLength)) {}

    /**
     * \param theLine The geometric \sa Line object that represents a straight-line
     * connection 
     * 
     * \param timeLength The time duration to traverse the straight trajectory
     * in units of \sa TimeType 
     * 
     * \param timeStep Time duration to folow eventually curved
     * trajectory in units of \sa TimesType 
     * 
     * \param initialV Initial velocity vector at
     * start of trajectory \param finalV Final velocity vector at start of trajectory
     */
    LineTrajectory(
        Line const& theLine,
        corsika::units::si::TimeType timeLength, // length of theLine (straight)
        corsika::units::si::TimeType timeStep,   // length of bend step (curved)
        const VelocityVec& initialV, const VelocityVec& finalV)
        : line_(theLine)
        , timeLength_(timeLength)
        , timeStep_(timeStep)
        , initialVelocity_(initialV)
        , finalVelocity_(finalV) {}

    const Line& GetLine() const { return line_; }
    Point GetPosition(double u) const { return line_.GetPosition(timeLength_ * u); }
    VelocityVec GetVelocity(double u) const {
      return initialVelocity_ * (1 - u) + finalVelocity_ * u;
    }
    Vector<corsika::units::si::dimensionless_d> GetDirection(double u) const {
      return GetVelocity(u).normalized();
    }

    Point GetPosition(double u) const;

    corsika::units::si::TimeType GetDuration() const;

    corsika::units::si::LengthType GetLength() const;

    corsika::units::si::LengthType GetDistance(corsika::units::si::TimeType t) const;

    void LimitEndTo(corsika::units::si::LengthType limit);

    auto NormalizedDirection() const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Trajectory.inl>
