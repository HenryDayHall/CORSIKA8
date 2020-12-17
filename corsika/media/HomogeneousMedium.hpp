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

  public:
    HomogeneousMedium(MassDensityType density, NuclearComposition const& nuclComp);

    MassDensityType getMassDensity(Point const&) const override;

    NuclearComposition const& getNuclearComposition() const override;

    GrammageType getIntegratedGrammage(Trajectory<Line> const&,
                                       LengthType to) const override;

    LengthType getArclengthFromGrammage(Trajectory<Line> const&,
                                        GrammageType grammage) const override;

  private:
    MassDensityType const density_;
    NuclearComposition const nuclComp_;    
  };

} // namespace corsika

#include <corsika/detail/media/HomogeneousMedium.inl>
