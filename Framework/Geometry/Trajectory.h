/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::geometry {

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
    LineTrajectory& operator=(const LineTrajectory&) = default;
    LineTrajectory(Line const& theLine, corsika::units::si::TimeType timeLength)
        : line_(theLine)
        , timeLength_(timeLength)
        , timeStep_(timeLength)
        , initialVelocity_(theLine.GetVelocity(timeLength * 0))
        , finalVelocity_(theLine.GetVelocity(timeLength * 1)) {}
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

    ///! duration along potentially bend trajectory
    corsika::units::si::TimeType GetDuration(double u = 1) const { return u * timeStep_; }

    ///! total length along potentially bend trajectory
    corsika::units::si::LengthType GetLength(double u = 1) const {
      using namespace corsika::units::si;
      if (timeLength_ == 0_s) return 0_m;
      return GetDistance(u) * timeStep_ / timeLength_;
    }

    ///! set new duration along potentially bend trajectory.
    void SetLength(corsika::units::si::LengthType limit) {
      SetDuration(line_.TimeFromArclength(limit));
    }

    ///! set new duration along potentially bend trajectory.
    //   Scale other properties by "limit/timeLength_"
    void SetDuration(corsika::units::si::TimeType limit) {
      using namespace corsika::units::si;
      if (timeStep_ == 0_s) {
        timeLength_ *= 0;
        SetFinalVelocity(GetVelocity(0));
        timeStep_ = limit;
      } else {
        const double scale = limit / timeStep_;
        timeLength_ *= scale;
        SetFinalVelocity(GetVelocity(scale));
        timeStep_ = limit;
      }
    }

  protected:
    ///! total length along straight trajectory
    corsika::units::si::LengthType GetDistance(double u) const {
      assert(u <= 1);
      assert(u >= 0);
      return line_.ArcLength(0 * corsika::units::si::second, u * timeLength_);
    }

    void SetFinalVelocity(const VelocityVec& v) { finalVelocity_ = v; }

  private:
    Line line_;
    corsika::units::si::TimeType timeLength_;
    corsika::units::si::TimeType timeStep_;
    VelocityVec initialVelocity_;
    VelocityVec finalVelocity_;
  };

  /**
   * \class LeapFrogTrajectory
   *
   *
   **/

  class LeapFrogTrajectory {

    using VelocityVec = Vector<corsika::units::si::SpeedType::dimension_type>;
    typedef corsika::geometry::Vector<corsika::units::si::magnetic_flux_density_d>
        MagneticFieldVector;

  public:
    LeapFrogTrajectory() = delete;
    LeapFrogTrajectory(const LeapFrogTrajectory&) = default;
    LeapFrogTrajectory(LeapFrogTrajectory&&) = default;
    LeapFrogTrajectory& operator=(const LeapFrogTrajectory&) = delete;
    LeapFrogTrajectory(const Point& pos, const VelocityVec& initialVelocity,
                       MagneticFieldVector Bfield,
                       const decltype(square(corsika::units::si::meter) /
                                      (square(corsika::units::si::second) *
                                       corsika::units::si::volt)) k,
                       corsika::units::si::TimeType timeStep) // leap-from total length
        : initialPosition_(pos)
        , initialVelocity_(initialVelocity)
        , initialDirection_(initialVelocity.normalized())
        , magneticfield_(Bfield)
        , k_(k)
        , timeStep_(timeStep) {}

    const Line GetLine(double u) const { return Line(GetPosition(u), GetVelocity(u)); }
    Point GetPosition(double u) const {
      Point position = initialPosition_ + initialVelocity_ * timeStep_ * u / 2;
      VelocityVec velocity =
          initialVelocity_ + initialVelocity_.cross(magneticfield_) * timeStep_ * u * k_;
      return position + velocity * timeStep_ * u / 2;
      //      auto steplength_true = steplength_true * (1.0 + double(direction.norm())) /
      //      2;
    }
    VelocityVec GetVelocity(double u) const {
      return initialVelocity_ +
             initialVelocity_.cross(magneticfield_) * timeStep_ * u * k_;
    }

    Vector<corsika::units::si::dimensionless_d> GetDirection(double u) const {
      return GetVelocity(u).normalized();
    }

    ///! duration along potentially bend trajectory
    corsika::units::si::TimeType GetDuration(double u = 1) const {
      return u * timeStep_ *
             (double(GetVelocity(u).norm() / initialVelocity_.norm()) + 1.0) / 2;
    }

    ///! total length along potentially bend trajectory
    corsika::units::si::LengthType GetLength(double u = 1) const {
      using namespace corsika::units::si;
      return timeStep_ * initialVelocity_.norm() * u;
    }

    ///! set new duration along potentially bend trajectory.
    void SetLength(corsika::units::si::LengthType limit) {
      using namespace corsika::units::si;
      if (initialVelocity_.norm() == 0_m / 1_s) SetDuration(0_s);
      SetDuration(limit / initialVelocity_.norm());
    }

    ///! set new duration along potentially bend trajectory.
    //   Scale other properties by "limit/timeLength_"
    void SetDuration(corsika::units::si::TimeType limit) {
      using namespace corsika::units::si;
      timeStep_ = limit;
    }

  private:
    Point initialPosition_;
    VelocityVec initialVelocity_;
    geometry::Vector<corsika::units::si::dimensionless_d> initialDirection_;
    MagneticFieldVector magneticfield_;
    decltype(square(corsika::units::si::meter) /
             (square(corsika::units::si::second) * corsika::units::si::volt)) k_;
    corsika::units::si::TimeType timeStep_;
  };

} // namespace corsika::geometry
