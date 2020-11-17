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

  template <typename T>
  SlidingPlanarExponential<T>::SlidingPlanarExponential(
      Point const& p0, units::si::MassDensityType rho0, units::si::LengthType lambda,
      NuclearComposition nuclComp, units::si::LengthType referenceHeight)
      : BaseExponential<SlidingPlanarExponential<T>>(p0, rho0, lambda)
      , nuclComp_(nuclComp)
      , referenceHeight_(referenceHeight) {}

  template <typename T>
  units::si::MassDensityType SlidingPlanarExponential<T>::getMassDensity(
      Point const& point) const {
    auto const height =
        (point - BaseExponential<SlidingPlanarExponential<T>>::point_).norm() -
        referenceHeight_;
    return BaseExponential<SlidingPlanarExponential<T>>::rho0_ *
           exp(BaseExponential<SlidingPlanarExponential<T>>::invLambda_ * height);
  }

  template <typename T>
  NuclearComposition const& SlidingPlanarExponential<T>::getNuclearComposition() const {
    return nuclComp_;
  }

  template <typename T>
  units::si::GrammageType SlidingPlanarExponential<T>::integratedGrammage(
      Trajectory<Line> const& line, units::si::LengthType l) const {
    auto const axis =
        (line.GetR0() - BaseExponential<SlidingPlanarExponential<T>>::point_).normalized();
    return BaseExponential<SlidingPlanarExponential<T>>::integratedGrammage(line, l,
                                                                            axis);
  }

  template <typename T>
  units::si::LengthType SlidingPlanarExponential<T>::arclengthFromGrammage(
      Trajectory<Line> const& line, units::si::GrammageType const grammage) const {
    auto const axis =
        (line.GetR0() - BaseExponential<SlidingPlanarExponential<T>>::point_).normalized();
    return BaseExponential<SlidingPlanarExponential<T>>::arclengthFromGrammage(
        line, grammage, axis);
  }

} // namespace corsika
