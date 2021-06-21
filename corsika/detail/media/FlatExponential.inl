/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/media/BaseExponential.hpp>
#include <corsika/media/NuclearComposition.hpp>

namespace corsika {

  template <typename T>
  inline FlatExponential<T>::FlatExponential(Point const& point,
                                             DirectionVector const& axis,
                                             MassDensityType rho, LengthType lambda,
                                             NuclearComposition const& nuclComp)
      : BaseExponential<FlatExponential<T>>(point, 0_m, rho, lambda)
      , axis_(axis)
      , nuclComp_(nuclComp) {}

  template <typename T>
  inline MassDensityType FlatExponential<T>::getMassDensity(Point const& point) const {
    return BaseExponential<FlatExponential<T>>::getMassDensity(
        (point - BaseExponential<FlatExponential<T>>::getAnchorPoint()).getNorm());
  }

  template <typename T>
  inline NuclearComposition const& FlatExponential<T>::getNuclearComposition() const {
    return nuclComp_;
  }

  template <typename T>
  inline GrammageType FlatExponential<T>::getIntegratedGrammage(
      BaseTrajectory const& line) const {
    return BaseExponential<FlatExponential<T>>::getIntegratedGrammage(line, axis_);
  }

  template <typename T>
  inline LengthType FlatExponential<T>::getArclengthFromGrammage(
      BaseTrajectory const& line, GrammageType grammage) const {
    return BaseExponential<FlatExponential<T>>::getArclengthFromGrammage(line, grammage,
                                                                         axis_);
  }

} // namespace corsika

#include <corsika/detail/media/BaseExponential.inl>
