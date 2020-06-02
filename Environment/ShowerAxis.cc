/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/ShowerAxis.h>

using namespace corsika::environment;
using namespace corsika::units::si;

GrammageType ShowerAxis::X(LengthType l) const {
  auto const fractionalBin = l / steplength_;
  int const lower = fractionalBin; // indices of nearest X support points
  auto const lambda = fractionalBin - lower;
  int const upper = lower + 1;

  if (upper >= steps) {
    throw std::runtime_error("shower axis too short, cannot extrapolate");
  } else if (lower < 0) {
    throw std::runtime_error("cannot extrapolate to points behind point of injection");
  }

  assert(0 <= lambda && lambda <= 1.);
  return (*X_)[lower] * lambda + (*X_)[upper] * (1 - lambda);
}

LengthType ShowerAxis::steplength() const { return steplength_; }

GrammageType ShowerAxis::maximumX() const { return *(X_->rbegin()); }

GrammageType ShowerAxis::minimumX() const { return *(X_->cbegin()); }

GrammageType ShowerAxis::projectedX(geometry::Point const& p) const {
  auto const projectedLength = (p - pointStart_).dot(axis_normalized_);
  return X(projectedLength);
}
