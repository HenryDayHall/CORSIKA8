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

  /**
   *
   * A Trajectory is a description of a momvement of an object in
   * three-dimensional space that describes the trajectory (connection
   * between two Points in space), as well as the direction of motion
   * at any given point.
   *
   * A Trajectory has a start `0` and an end `1`, where
   * e.g. getPosition(0) returns the start point and getDirection(1)
   * the direction of motion at the end. Values outside 0...1 are not
   * defined.
   *
   * A Trajectory has a length in [m], getLength, a duration in [s], getDuration.
   *
   * Note: so far it is assumed that the speed (d|vec{r}|/dt) between
   * start and end does not change and is constant for the entire
   * Trajectory.
   *
   **/

  class LineTrajectory {

  public:
    LineTrajectory() = delete;
    LineTrajectory(LineTrajectory const&) = default;
    LineTrajectory(LineTrajectory&&) = default;
    LineTrajectory& operator=(LineTrajectory const&) = delete;

    /**
     * \param theLine The geometric \sa Line object that represents a straight-line
     * connection
     *
     * \param timeLength The time duration to traverse the straight trajectory
     * in units of \sa TimeType
     */
    LineTrajectory(Line const& theLine, TimeType timeLength)
        : line_(theLine)
        , timeLength_(timeLength)
        , timeStep_(timeLength)
        , initialVelocity_(theLine.getVelocity(TimeType::zero()))
        , finalVelocity_(theLine.getVelocity(timeLength)) {}

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
    LineTrajectory(Line const& theLine,
                   TimeType const timeLength, // length of theLine (straight)
                   TimeType const timeStep,   // length of bend step (curved)
                   VelocityVector const& initialV, VelocityVector const& finalV)
        : line_(theLine)
        , timeLength_(timeLength)
        , timeStep_(timeStep)
        , initialVelocity_(initialV)
        , finalVelocity_(finalV) {}

    Line const& getLine() const { return line_; }

    Point getPosition(double const u) const { return line_.getPosition(timeLength_ * u); }

    VelocityVector getVelocity(double const u) const;

    DirectionVector getDirection(double const u) const {
      return getVelocity(u).normalized();
    }

    ///! duration along potentially bend trajectory
    TimeType getDuration(double const u = 1) const;

    ///! total length along potentially bend trajectory
    LengthType getLength(double const u = 1) const;

    ///! set new duration along potentially bend trajectory.
    void setLength(LengthType const limit);

    ///! set new duration along potentially bend trajectory.
    //   Scale other properties by "limit/timeLength_"
    void setDuration(TimeType const limit);

  protected:
    ///! total length along straight trajectory
    LengthType getDistance(double const u) const;

    void setFinalVelocity(VelocityVector const& v) { finalVelocity_ = v; }

  private:
    Line line_;
    TimeType timeLength_; ///! length of straight step (shortest connecting line)
    TimeType timeStep_;   ///! length of bend step (curved)
    VelocityVector initialVelocity_;
    VelocityVector finalVelocity_;
  };

  /**
   * The LeapFrogTrajectory stores information on one leap-frog step.
   *
   * The leap-frog algorithm uses a half-step and is used in magnetic
   * field tracking. The LeapFrogTrajectory will solve the leap-frog
   * algorithm equation for a given constant $k$ that has to be
   * specified during construction (essentially fixing the magnetic
   * field). Thus, different steps (length) can be dynamically
   * generated here. The velocity vector will correctly point into the
   * direction as calculated by the algorithm for any steplength, or
   * intermediate position.
   *
   **/

  class LeapFrogTrajectory {

  public:
    LeapFrogTrajectory() = delete;
    LeapFrogTrajectory(LeapFrogTrajectory const&) = default;
    LeapFrogTrajectory(LeapFrogTrajectory&&) = default;
    LeapFrogTrajectory& operator=(LeapFrogTrajectory const&) = delete;

    LeapFrogTrajectory(Point const& pos, VelocityVector const& initialVelocity,
                       MagneticFieldVector const& Bfield,
                       decltype(square(meter) / (square(second) * volt)) const k,
                       TimeType const timeStep) // leap-from total length
        : initialPosition_(pos)
        , initialVelocity_(initialVelocity)
        , initialDirection_(initialVelocity.normalized())
        , magneticfield_(Bfield)
        , k_(k)
        , timeStep_(timeStep) {}

    Line const getLine() const;

    Point getPosition(double const u) const;

    VelocityVector getVelocity(double const u) const;

    DirectionVector getDirection(double const u) const;

    ///! duration along potentially bend trajectory
    TimeType getDuration(double const u = 1) const;

    ///! total length along potentially bend trajectory
    LengthType getLength(double const u = 1) const;

    ///! set new duration along potentially bend trajectory.
    void setLength(LengthType const limit);

    ///! set new duration along potentially bend trajectory.
    //   Scale other properties by "limit/timeLength_"
    void setDuration(TimeType const limit);

  private:
    Point initialPosition_;
    VelocityVector initialVelocity_;
    DirectionVector initialDirection_;
    MagneticFieldVector magneticfield_;
    decltype(square(meter) / (square(second) * volt)) k_;
    TimeType timeStep_;
  };

  /*

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
  */

} // namespace corsika

#include <corsika/detail/framework/geometry/Trajectory.inl>
