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

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {

  inline bool Plane::isAbove(Point const& vP) const {
    return normal_.dot(vP - center_) > LengthType::zero();
  }

  inline LengthType Plane::getDistanceTo(Point const& vP) const {
    return (normal_ * (vP - center_).dot(normal_)).getNorm();
  }

  inline Point const& Plane::getCenter() const { return center_; }

  inline Plane::DimLessVec const& Plane::getNormal() const { return normal_; }

} // namespace corsika
