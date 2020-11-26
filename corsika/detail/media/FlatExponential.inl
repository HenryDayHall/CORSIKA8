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

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/media/BaseExponential.hpp>
#include <corsika/media/NuclearComposition.hpp>

namespace corsika {

  template <typename T>
  FlatExponential<T>::FlatExponential(Point const& point,
                                      Vector<units::si::dimensionless_d> const& axis,
                                      units::si::MassDensityType rho,
                                      units::si::LengthType lambda,
                                      NuclearComposition nuclComp)
      : BaseExponential<FlatExponential<T>>(point, rho, lambda)
      , axis_(axis)
      , nuclComp_(nuclComp) {}

  template <typename T>
  units::si::MassDensityType FlatExponential<T>::getMassDensity(
      Point const& point) const {
    return BaseExponential<FlatExponential<T>>::rho0_ *
           exp(BaseExponential<FlatExponential<T>>::invLambda_ *
               (point - BaseExponential<FlatExponential<T>>::point_).dot(axis_));
  }

  template <typename T>
  NuclearComposition const& FlatExponential<T>::getNuclearComposition() const {
    return nuclComp_;
  }

  template <typename T>
  GrammageType FlatExponential<T>::integratedGrammage(
      Trajectory<Line> const& line, units::si::LengthType to) const {
    return BaseExponential<FlatExponential<T>>::integratedGrammage(line, to, axis_);
  }

  template <typename T>
  LengthType FlatExponential<T>::getArclengthFromGrammage(
      Trajectory<Line> const& line, units::si::GrammageType grammage) const {
    return BaseExponential<FlatExponential<T>>::arclengthFromGrammage(line, grammage,
                                                                      axis_);
  }

} // namespace corsika

#include <corsika/detail/media/BaseExponential.inl>
