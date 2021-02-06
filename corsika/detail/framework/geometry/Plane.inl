/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <sstream>

namespace corsika {

  inline bool Plane::isAbove(Point const& vP) const {
    return normal_.dot(vP - center_) > LengthType::zero();
  }

  inline LengthType Plane::getDistanceTo(Point const& vP) const {
    return (normal_ * (vP - center_).dot(normal_)).getNorm();
  }

  inline Point const& Plane::getCenter() const { return center_; }

  inline DirectionVector const& Plane::getNormal() const { return normal_; }

  inline std::string Plane::asString() const {
    std::ostringstream txt;
    txt << "center=" << center_ << ", normal=" << normal_;
    return txt.str();
  }

} // namespace corsika
