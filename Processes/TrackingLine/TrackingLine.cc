
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

  std::optional<std::pair<corsika::units::si::TimeType, corsika::units::si::TimeType>>
  TimeOfIntersection(corsika::geometry::Line const& line,
                                                      geometry::Sphere const& sphere) {
    auto const delta = line.GetR0() - sphere.GetCenter();
    auto const v = line.GetV0();
    auto const vSqNorm = v.squaredNorm();
    auto const R = sphere.GetRadius();

    auto const vDotDelta = v.dot(delta);
    auto const discriminant =
        vDotDelta * vDotDelta - vSqNorm * (delta.squaredNorm() - R * R);

    //~ std::cout << "discriminant: " << discriminant << std::endl;
    //~ std::cout << "alpha: " << alpha << std::endl;
    //~ std::cout << "beta: " << beta << std::endl;

    if (discriminant.magnitude() > 0) {
      auto const sqDisc = sqrt(discriminant);
      auto const invDenom = 1 / vSqNorm;
      return std::make_pair((vDotDelta - sqDisc) * invDenom,
                            (vDotDelta + sqDisc) * invDenom);
    } else {
      return {};
    }
  }
} // namespace corsika::process::tracking_line

