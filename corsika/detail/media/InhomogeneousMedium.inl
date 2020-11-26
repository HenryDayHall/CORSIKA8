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
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/media/NuclearComposition.hpp>

namespace corsika {

  template <typename T, typename TDensityFunction>
  template <typename... TArgs>
  InhomogeneousMedium<T, TDensityFunction>::InhomogeneousMedium(
      NuclearComposition const& nuclComp, TArgs&&... rhoTArgs)
      : nuclComp_(nuclComp)
      , densityFunction_(rhoTArgs...){}

  template <typename T, typename TDensityFunction>
  units::si::MassDensityType InhomogeneousMedium<T, TDensityFunction>::getMassDensity(
      Point const& point) const {
    return densityFunction_.evaluateAt(point);
  }

  template <typename T, typename TDensityFunction>
  NuclearComposition const&
  InhomogeneousMedium<T, TDensityFunction>::getNuclearComposition() const {
    return nuclComp_;
  }

  template <typename T, typename TDensityFunction>
  GrammageType InhomogeneousMedium<T, TDensityFunction>::integratedGrammage(
      Trajectory<Line> const& line, units::si::LengthType to) const {
    return densityFunction_.integrateGrammage(line, to);
  }

  template <typename T, typename TDensityFunction>
  LengthType InhomogeneousMedium<T, TDensityFunction>::getArclengthFromGrammage(
      Trajectory<Line> const& line, units::si::GrammageType grammage) const {
    return densityFunction_.getArclengthFromGrammage(line, grammage);
  }

} // namespace corsika
