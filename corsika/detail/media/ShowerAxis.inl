/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/logging/Logging.hpp>

#include <string>

namespace corsika {

  template <typename TEnvModel>
  ShowerAxis::ShowerAxis(Point const& pStart, Vector<length_d> const& length,
                         Environment<TEnvModel> const& env, int steps)
      : pointStart_(pStart)
      , length_(length)
      , max_length_(length_.getNorm())
      , steplength_(max_length_ / steps)
      , axis_normalized_(length / max_length_)
      , X_(steps + 1) {
    auto const* const universe = env.getUniverse().get();

    auto rho = [pStart, length, universe](double x) {
      auto const p = pStart + length * x;
      auto const* node = universe->getContainingNode(p);
      return node->getModelProperties().getMassDensity(p).magnitude();
    };

    double error;
    int k = 0;
    X_[0] = GrammageType::zero();
    auto sum = GrammageType::zero();

    for (int i = 1; i <= steps; ++i) {
      auto const x_prev = (i - 1.) / steps;
      auto const d_prev = max_length_ * x_prev;
      auto const x = double(i) / steps;
      auto const r = boost::math::quadrature::gauss_kronrod<double, 15>::integrate(
          rho, x_prev, x, 15, 1e-9, &error);
      auto const result =
          MassDensityType(phys::units::detail::magnitude_tag, r) * max_length_;

      sum += result;
      X_[i] = sum;

      for (; sum > k * X_binning_; ++k) {
        d_.emplace_back(d_prev + k * X_binning_ * steplength_ / result);
      }
    }

    assert(std::is_sorted(X_.cbegin(), X_.cend()));
    assert(std::is_sorted(d_.cbegin(), d_.cend()));
  }

  GrammageType ShowerAxis::getX(LengthType l) const {
    auto const fractionalBin = l / steplength_;
    int const lower = fractionalBin; // indices of nearest X support points
    auto const lambda = fractionalBin - lower;
    decltype(X_.size()) const upper = lower + 1;

    if (lower < 0) {
      CORSIKA_LOG_ERROR("cannot extrapolate to points behind point of injection l={} m",
                        l / 1_m);
      throw std::runtime_error("cannot extrapolate to points behind point of injection");
    }

    if (upper >= X_.size()) {
      const std::string err =
          fmt::format("shower axis too short, cannot extrapolate (l / max_length_ = {} )",
                      l / max_length_);
      CORSIKA_LOG_ERROR(err);
      throw std::runtime_error(err.c_str());
    }

    assert(0 <= lambda && lambda <= 1.);

    CORSIKA_LOG_TRACE("ShowerAxis::getX l={} m, lower={}, lambda={}, upper={}", l / 1_m,
                      lower, lambda, upper);

    // linear interpolation between getX[lower] and X[upper]
    return X_[upper] * lambda + X_[lower] * (1 - lambda);
  }

  LengthType ShowerAxis::getSteplength() const { return steplength_; }

  GrammageType ShowerAxis::getMaximumX() const { return *X_.rbegin(); }

  GrammageType ShowerAxis::getMinimumX() const { return GrammageType::zero(); }

  GrammageType ShowerAxis::getProjectedX(Point const& p) const {
    auto const projectedLength = (p - pointStart_).dot(axis_normalized_);
    return getX(projectedLength);
  }

  Vector<dimensionless_d> const& ShowerAxis::getDirection() const {
    return axis_normalized_;
  }

  Point const& ShowerAxis::getStart() const { return pointStart_; }

} // namespace corsika