/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/TrackingLine.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/media/Environment.hpp>

#include <limits>
#include <stdexcept>
#include <utility>

namespace corsika::tracking_line {

  std::optional<std::pair<TimeType, TimeType>> TimeOfIntersection(
      corsika::Line const& line, corsika::Sphere const& sphere) {
    auto const delta = line.getStartPoint() - sphere.getCenter();
    auto const v = line.getVelocity();
    auto const vSqNorm =
        v.getSquaredNorm(); // todo: get rid of this by having V0 normalized always
    auto const R = sphere.getRadius();

    auto const vDotDelta = v.dot(delta);
    auto const discriminant =
        vDotDelta * vDotDelta - vSqNorm * (delta.getSquaredNorm() - R * R);

    if (discriminant.magnitude() > 0) {
      auto const sqDisc = sqrt(discriminant);
      auto const invDenom = 1 / vSqNorm;
      return std::make_pair((-vDotDelta - sqDisc) * invDenom,
                            (-vDotDelta + sqDisc) * invDenom);
    } else {
      return {};
    }
  }

  TimeType getTimeOfIntersection(Line const& vLine, Plane const& vPlane) {

    auto const delta = vPlane.getCenter() - vLine.getStartPoint();
    auto const v = vLine.getVelocity();
    auto const n = vPlane.getNormal();
    auto const c = n.dot(v);

    if (c.magnitude() == 0) {
      return std::numeric_limits<TimeType::value_type>::infinity() * 1_s;
    } else {
      return n.dot(delta) / c;
    }
  }

} // namespace corsika::tracking_line
