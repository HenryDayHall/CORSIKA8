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

    Line getLine() const;

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

} // namespace corsika

#include <corsika/detail/framework/geometry/LeapFrogTrajectory.inl>
