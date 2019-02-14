
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/tracking_line/TrackingLine.h>

#include <corsika/environment/Environment.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/QuantityVector.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Vector.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

using namespace corsika;

namespace corsika::process::tracking_line {

  template <class Stack, class Trajectory>
  std::optional<std::pair<corsika::units::si::TimeType, corsika::units::si::TimeType>>
  TrackingLine<Stack, Trajectory>::TimeOfIntersection(corsika::geometry::Line const& line,
                                                      geometry::Sphere const& sphere) {
    using namespace corsika::units::si;
    auto const& cs = fEnvironment.GetCoordinateSystem();
    geometry::Point const origin(cs, 0_m, 0_m, 0_m);

    auto const r0 = (line.GetR0() - origin);
    auto const v0 = line.GetV0();
    auto const c0 = (sphere.GetCenter() - origin);

    auto const alpha = r0.dot(v0) - 2 * v0.dot(c0);
    auto const beta = c0.squaredNorm() + r0.squaredNorm() + 2 * c0.dot(r0) -
                      sphere.GetRadius() * sphere.GetRadius();

    auto const discriminant = alpha * alpha - 4 * beta * v0.squaredNorm();

    //~ std::cout << "discriminant: " << discriminant << std::endl;
    //~ std::cout << "alpha: " << alpha << std::endl;
    //~ std::cout << "beta: " << beta << std::endl;

    if (discriminant.magnitude() > 0) {
      (-alpha - sqrt(discriminant)) / (2 * v0.squaredNorm());
      return std::make_pair((-alpha - sqrt(discriminant)) / (2 * v0.squaredNorm()),
                            (-alpha + sqrt(discriminant)) / (2 * v0.squaredNorm()));
    } else {
      return {};
    }
  }

  template <class Stack, class Trajectory>
  TrackingLine<Stack, Trajectory>::TrackingLine(
      corsika::environment::Environment const& pEnv)
      : fEnvironment(pEnv) {}

} // namespace corsika::process::tracking_line

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
using namespace corsika::setup;
template class corsika::process::tracking_line::TrackingLine<setup::Stack,
                                                             setup::Trajectory>;

#include "testTrackingLineStack.h"
template class corsika::process::tracking_line::TrackingLine<DummyStack,
                                                             setup::Trajectory>;
