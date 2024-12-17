/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the 3-clause BSD license.
 * See file LICENSE for a full version of the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/BaseTrajectory.hpp>

namespace corsika {

  /**
   * A general inhomogeneous medium. The mass density distribution TDensityFunction must
   * be a \f$C^2\f$-function.
   */

  template <typename T, typename TDensityFunction>
  class InhomogeneousMedium : public T {

  public:
    template <typename... TArgs>
    InhomogeneousMedium(NuclearComposition const& nuclComp, TArgs&&... rhoTArgs);

    MassDensityType getMassDensity(Point const& point) const override;

    NuclearComposition const& getNuclearComposition() const override;

    GrammageType getIntegratedGrammage(BaseTrajectory const& line) const override;

    LengthType getArclengthFromGrammage(BaseTrajectory const& pLine,
                                        GrammageType grammage) const override;

  private:
    NuclearComposition const nuclComp_;
    TDensityFunction const densityFunction_;
  };

} // namespace corsika

#include <corsika/detail/media/InhomogeneousMedium.inl>
