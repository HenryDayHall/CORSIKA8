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
  class Box : public IVolume {

  public:
    // a CoordinateSystemPtr to specify the orintation of coordinate
    Box(Point const& center, CoordinateSystemPtr cs, LengthType const x,
        LengthType const y, LengthType const z)
        : center_(center)
        , cs_(make_translation(cs, center.getCoordinates(cs)))
        , x_(x)
        , y_(y)
        , z_(z) {}

    Box(Point const& center, CoordinateSystemPtr cs, LengthType const side)
        : center_(center)
        , cs_(make_translation(cs, center.getCoordinates(cs)))
        , x_(side / 2)
        , y_(side / 2)
        , z_(side / 2) {}

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
