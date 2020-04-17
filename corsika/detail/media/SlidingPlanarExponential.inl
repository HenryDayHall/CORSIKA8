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

#include <corsika/media/SlidingPlanarExponential.hpp>

namespace corsika {

  template <class T>
  units::si::MassDensityType SlidingPlanarExponential<T>::GetMassDensity(
      Point const& p) const {
    auto const height = (p - Base::fP0).norm() - referenceHeight_;
    return Base::fRho0 * exp(Base::fInvLambda * height);
  }

  template <class T>
  units::si::GrammageType SlidingPlanarExponential<T>::IntegratedGrammage(
      Trajectory<Line> const& line, units::si::LengthType l) const {
    auto const axis = (line.GetR0() - Base::fP0).normalized();
    return Base::IntegratedGrammage(line, l, axis);
  }

  template <class T>
  units::si::LengthType SlidingPlanarExponential<T>::ArclengthFromGrammage(
      Trajectory<Line> const& line, units::si::GrammageType grammage) const {
    auto const axis = (line.GetR0() - Base::fP0).normalized();
    return Base::ArclengthFromGrammage(line, grammage, axis);
  }

} // namespace corsika