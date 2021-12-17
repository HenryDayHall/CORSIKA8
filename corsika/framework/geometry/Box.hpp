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
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/IVolume.hpp>

namespace corsika {

  /**
   * Describes a sphere in space
   *
   *  The center point and the orintation of the Box is set by
   *  a CoordinateSystemPtr at construction.
   **/
  class Box : public IVolume {

  public:
    Box(CoordinateSystemPtr cs, LengthType const x, LengthType const y,
        LengthType const z);

    Box(CoordinateSystemPtr cs, LengthType const side);

    //! returns true if the Point p is within the sphere
    bool contains(Point const& p) const override;

    Point const& getCenter() const { return center_; };
    CoordinateSystemPtr const getCoordinateSystem() const { return cs_; }

    LengthType const getX() const { return x_; }
    LengthType const getY() const { return y_; }
    LengthType const getZ() const { return z_; }

    std::string asString() const;

    template <typename TDim>
    void rotate(QuantityVector<TDim> const& axis, double const angle);

  protected:
    Point center_;
    CoordinateSystemPtr cs_; // local coordinate system with center_ in coordinate (0, 0,
                             // 0) and user defined orientation
    LengthType x_;
    LengthType y_;
    LengthType z_;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Box.inl>
