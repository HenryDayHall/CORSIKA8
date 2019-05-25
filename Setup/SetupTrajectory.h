/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_setup_setuptrajectory_h_
#define _corsika_setup_setuptrajectory_h_

#include <corsika/geometry/Helix.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Trajectory.h>

#include <corsika/units/PhysicalUnits.h>

// #include <variant>

namespace corsika::setup {

  /// definition of Trajectory base class, to be used in tracking and cascades
  typedef corsika::geometry::Trajectory<corsika::geometry::Line> Trajectory;

  /*
  typedef std::variant<std::monostate, corsika::geometry::Trajectory<Line>,
                       corsika::geometry::Trajectory<Helix>>
                       Trajectory;

  /// helper visitor to modify Particle by moving along Trajectory
  template <typename Particle>
  class ParticleUpdate {

    Particle& fP;

  public:
    ParticleUpdate(Particle& p)
        : fP(p) {}
    void operator()(std::monostate const&) {}

    template <typename T>
    void operator()(T const& trajectory) {
      fP.SetPosition(trajectory.GetPosition(1));
    }
  };

  /// helper visitor to modify Particle by moving along Trajectory
  class GetDuration {
  public:
    corsika::units::si::TimeType operator()(std::monostate const&) {
      return 0 * corsika::units::si::second;
    }
    template <typename T>
    corsika::units::si::TimeType operator()(T const& trajectory) {
      return trajectory.GetDuration();
    }
  };
  */
} // namespace corsika::setup

#endif
