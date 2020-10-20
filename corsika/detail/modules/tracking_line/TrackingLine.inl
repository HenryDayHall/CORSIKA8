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

#include <corsika/modules/tracking_line/TrackingLine.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/media/Environment.hpp>

#include <limits>
#include <stdexcept>
#include <utility>

namespace corsika::tracking_line {

  std::optional<std::pair<units::si::TimeType, units::si::TimeType>> TimeOfIntersection(
      corsika::Line const& line, corsika::Sphere const& sphere) {
    auto const delta = line.GetR0() - sphere.GetCenter();
    auto const v = line.GetV0();
    auto const vSqNorm =
        v.squaredNorm(); // todo: get rid of this by having V0 normalized always
    auto const R = sphere.GetRadius();

    auto const vDotDelta = v.dot(delta);
    auto const discriminant =
        vDotDelta * vDotDelta - vSqNorm * (delta.squaredNorm() - R * R);

    if (discriminant.magnitude() > 0) {
      auto const sqDisc = sqrt(discriminant);
      auto const invDenom = 1 / vSqNorm;
      return std::make_pair((-vDotDelta - sqDisc) * invDenom,
                            (-vDotDelta + sqDisc) * invDenom);
    } else {
      return {};
    }
  }

  units::si::TimeType TimeOfIntersection(Line const& vLine, Plane const& vPlane) {

    using namespace units::si;

    auto const delta = vPlane.GetCenter() - vLine.GetR0();
    auto const v = vLine.GetV0();
    auto const n = vPlane.GetNormal();
    auto const c = n.dot(v);

    if (c.magnitude() == 0) {
      return std::numeric_limits<units::si::TimeType::value_type>::infinity() * 1_s;
    } else {
      return n.dot(delta) / c;
    }
  }
  
} // namespace corsika::tracking_line
