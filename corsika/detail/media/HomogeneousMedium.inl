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

  template <typename T>
  HomogeneousMedium<T>::HomogeneousMedium(units::si::MassDensityType density,
                                          NuclearComposition nuclComp)
      : density_(density)
      , nuclComp_(nuclComp) {}

  template <typename T>
  units::si::MassDensityType HomogeneousMedium<T>::getMassDensity(Point const&) const {
    return density_;
  }
  template <typename T>
  NuclearComposition const& HomogeneousMedium<T>::getNuclearComposition() const {
    return nuclComp_;
  }

  template <typename T>
  GrammageType HomogeneousMedium<T>::integratedGrammage(
      Trajectory<Line> const&, units::si::LengthType to) const {
    return to * density_;
  }

  template <typename T>
  LengthType HomogeneousMedium<T>::getArclengthFromGrammage(
      Trajectory<Line> const&, units::si::GrammageType grammage) const {
    return grammage / density_;
  }
} // namespace corsika
