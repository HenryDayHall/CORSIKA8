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
 * A general inhomogeneous medium. The mass density distribution TDensityFunction must be
 * a \f$C^2\f$-function.
 */

namespace corsika {

  template <typename T, typename TDensityFunction>
  class InhomogeneousMedium : public T {
    NuclearComposition const nuclComp_;
    TDensityFunction const densityFunction_;

  public:
    template <typename... TArgs>
    InhomogeneousMedium(NuclearComposition nuclComp, TArgs&&... rhoTArgs);

    units::si::MassDensityType getMassDensity(Point const& point) const override;

    NuclearComposition const& getNuclearComposition() const override;

    units::si::GrammageType integratedGrammage(Trajectory<Line> const& line,
                                               units::si::LengthType to) const override;

    units::si::LengthType arclengthFromGrammage(
        Trajectory<Line> const& pLine, units::si::GrammageType grammage) const override;
  };

} // namespace corsika

#include <corsika/detail/media/InhomogeneousMedium.inl>
