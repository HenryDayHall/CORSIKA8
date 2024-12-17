/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  inline bool Sphere::contains(Point const& p) const {
    return radius_ * radius_ > (center_ - p).getSquaredNorm();
  }

  inline Point const& Sphere::getCenter() const { return center_; }

  inline void Sphere::setCenter(Point const& p) { center_ = p; }

  inline LengthType Sphere::getRadius() const { return radius_; }

  inline void Sphere::setRadius(LengthType const r) { radius_ = r; }

  inline CoordinateSystemPtr const Sphere::getCoordinateSystem() const {
    return center_.getCoordinateSystem();
  }

  inline std::string Sphere::asString() const {
    std::ostringstream txt;
    txt << "center=" << center_ << ", radius=" << radius_;
    return txt.str();
  }

} // namespace corsika
