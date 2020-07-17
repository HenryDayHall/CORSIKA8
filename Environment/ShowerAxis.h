/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

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

namespace corsika::environment {
  class ShowerAxis {
  public:
    template <typename TEnvModel>
    ShowerAxis(geometry::Point const& pStart,
               geometry::Vector<units::si::length_d> length,
               environment::Environment<TEnvModel> const& env, int steps = 10'000)
        : pointStart_(pStart)
        , length_(length)
        , max_length_(length_.norm())
        , steplength_(max_length_ / steps)
        , axis_normalized_(length / max_length_)
        , X_(steps + 1) {
      auto const* const universe = env.GetUniverse().get();

      auto rho = [pStart, length, universe](double x) {
        auto const p = pStart + length * x;
        auto const* node = universe->GetContainingNode(p);
        return node->GetModelProperties().GetMassDensity(p).magnitude();
      };

      double error;
      int k = 0;
      X_[0] = units::si::GrammageType::zero();
      auto sum = units::si::GrammageType::zero();

      for (int i = 1; i <= steps; ++i) {
        auto const x_prev = (i - 1.) / steps;
        auto const d_prev = max_length_ * x_prev;
        auto const x = double(i) / steps;
        auto const r = boost::math::quadrature::gauss_kronrod<double, 15>::integrate(
            rho, x_prev, x, 15, 1e-9, &error);
        auto const result =
            units::si::MassDensityType(phys::units::detail::magnitude_tag, r) *
            max_length_;

        sum += result;
        X_[i] = sum;

        for (; sum > k * X_binning_; ++k) {
          d_.emplace_back(d_prev + k * X_binning_ * steplength_ / result);
        }
      }

      assert(std::is_sorted(X_.cbegin(), X_.cend()));
      assert(std::is_sorted(d_.cbegin(), d_.cend()));
    }

    units::si::LengthType steplength() const;

    units::si::GrammageType maximumX() const;

    units::si::GrammageType minimumX() const;

    units::si::GrammageType projectedX(geometry::Point const& p) const;

    units::si::GrammageType X(units::si::LengthType) const;

    geometry::Vector<units::si::dimensionless_d> const& GetDirection() const;

    geometry::Point const& GetStart() const;

  private:
    geometry::Point const pointStart_;
    geometry::Vector<units::si::length_d> const length_;
    units::si::LengthType const max_length_, steplength_;
    geometry::Vector<units::si::dimensionless_d> const axis_normalized_;
    std::vector<units::si::GrammageType> X_;

    // for storing the lengths corresponding to equidistant X values
    units::si::GrammageType const X_binning_ = std::invoke([]() {
      using namespace units::si;
      return 1_g / 1_cm / 1_cm;
    });
    std::vector<units::si::LengthType> d_;
  };
} // namespace corsika::environment
