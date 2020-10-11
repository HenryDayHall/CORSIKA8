/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/ShowerAxis.h>
#include <corsika/logging/Logging.h>

#include <string>

using namespace corsika::environment;
using namespace corsika::units::si;
using namespace corsika;

GrammageType ShowerAxis::X(LengthType l) const {
  auto const fractionalBin = l / steplength_;
  int const lower = fractionalBin; // indices of nearest X support points
  auto const lambda = fractionalBin - lower;
  decltype(X_.size()) const upper = lower + 1;

  if (lower < 0) {
    C8LOG_ERROR("cannot extrapolate to points behind point of injection l={} m", l/1_m);
    throw std::runtime_error("cannot extrapolate to points behind point of injection");
  }

  if (upper >= X_.size()) { 
    const std::string err = fmt::format("shower axis too short, cannot extrapolate (l / max_length_ = {} )",
				   l / max_length_);
    C8LOG_ERROR(err);
    throw std::runtime_error(err.c_str());
  }

  assert(0 <= lambda && lambda <= 1.);

  C8LOG_TRACE("ShowerAxis::X l={} m, lower={}, lambda={}, upper={}", l/1_m, lower, lambda, upper);

  // linear interpolation between X[lower] and X[upper]
  return X_[upper] * lambda + X_[lower] * (1 - lambda);
}

LengthType ShowerAxis::steplength() const { return steplength_; }

GrammageType ShowerAxis::maximumX() const { return *X_.rbegin(); }

GrammageType ShowerAxis::minimumX() const { return GrammageType::zero(); }

GrammageType ShowerAxis::projectedX(geometry::Point const& p) const {
  auto const projectedLength = (p - pointStart_).dot(axis_normalized_);
  return X(projectedLength);
}

geometry::Vector<units::si::dimensionless_d> const& ShowerAxis::GetDirection() const {
  return axis_normalized_;
}

geometry::Point const& ShowerAxis::GetStart() const { return pointStart_; }
