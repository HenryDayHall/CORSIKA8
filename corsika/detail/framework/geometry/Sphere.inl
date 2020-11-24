/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  bool Sphere::isInside(Point const& p) const {
    return radius_ * radius_ > (center_ - p).getSquaredNorm();
  }

  Point const& Sphere::getCenter() const { return center_; }

  LengthType Sphere::getRadius() const { return radius_; }

} // namespace corsika
