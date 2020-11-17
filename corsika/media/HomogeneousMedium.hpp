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

/**
 * a homogeneous medium
 */

namespace corsika {

  template <typename T>
  class HomogeneousMedium : public T {
    units::si::MassDensityType const density_;
    NuclearComposition const nuclComp_;

  public:
    HomogeneousMedium(units::si::MassDensityType density, NuclearComposition nuclComp);

      units::si::MassDensityType getMassDensity(Point const&) const override;

    NuclearComposition const& getNuclearComposition() const override;

    units::si::GrammageType integratedGrammage(
        Trajectory<Line> const&,
        units::si::LengthType to) const override;

    units::si::LengthType arclengthFromGrammage(
        Trajectory<Line> const&,
        units::si::GrammageType grammage) const override;
  };

} // namespace corsika

#include <corsika/detail/media/HomogeneousMedium.inl>
