
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_processes_TrackingLine_h_
#define _include_corsika_processes_TrackingLine_h_

#include <corsika/units/PhysicalUnits.h>
#include <map> // for std::pair
#include <optional>

namespace corsika::environment {
  class Environment;
}
namespace corsika::geometry {
  class Line;
  class Sphere;
} // namespace corsika::geometry

namespace corsika::process {

  namespace tracking_line {

    template <typename Stack, typename Trajectory>
    class TrackingLine { //

      using Particle = typename Stack::StackIterator;

      corsika::environment::Environment const& fEnvironment;

    public:
      std::optional<std::pair<corsika::units::si::TimeType, corsika::units::si::TimeType>>
      TimeOfIntersection(corsika::geometry::Line const& line,
                         geometry::Sphere const& sphere);

      TrackingLine(corsika::environment::Environment const& pEnv);

      Trajectory GetTrack(Particle const& p);
    };

  } // namespace tracking_line

} // namespace corsika::process

#endif
