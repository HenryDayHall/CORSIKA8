/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

#include <iostream>

#include <boost/math/quadrature/gauss_kronrod.hpp>

namespace corsika {

  /**
   * \class ShowerAxis
   *
   * The environment::ShowerAxis is created from a Point and
   * a Vector and inside an Environment. It internally uses
   * a table with steps=10000 (default) rows for interpolation.
   *
   * The shower axis can convert location in the shower into a
   * projected grammage along the shower axis.
   *
   **/

    ///\todo documentation needs update ...
  class ShowerAxis {
  public:
    template <typename TEnvModel>
    ShowerAxis(corsika::Point const& pStart,
               corsika::Vector<units::si::length_d> length,
               Environment<TEnvModel> const& env, int steps = 10'000);

    units::si::LengthType steplength() const;

    units::si::GrammageType maximumX() const;

    units::si::GrammageType minimumX() const;

    units::si::GrammageType projectedX(Point const& p) const;

    GrammageType getX(LengthType) const;

    Vector<units::si::dimensionless_d> const& getDirection() const;

    Point const& getStart() const;

  private:
    Point const pointStart_;
    Vector<units::si::length_d> const length_;
    units::si::LengthType const max_length_, steplength_;
    Vector<units::si::dimensionless_d> const axis_normalized_;
    std::vector<units::si::GrammageType> X_;

    // for storing the lengths corresponding to equidistant X values
    units::si::GrammageType const X_binning_ = std::invoke([]() {
      using namespace units::si;
      return 1_g / 1_cm / 1_cm;
    });
    std::vector<units::si::LengthType> d_;
  };
} // namespace corsika

#include <corsika/detail/media/ShowerAxis.inl>