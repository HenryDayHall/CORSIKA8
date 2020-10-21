/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/NuclearComposition.hpp>

namespace corsika {

  template <class T>
  class SlidingPlanarExponential : public BaseExponential<SlidingPlanarExponential<T>>,
                                   public T {

    NuclearComposition const nuclComp_;
    LengthType const referenceHeight_;

    using Base = BaseExponential<SlidingPlanarExponential<T>>;

  public:
    SlidingPlanarExponential(Point const& p0, MassDensityType rho0, LengthType lambda,
                             NuclearComposition nuclComp,
                             LengthType referenceHeight = LengthType::zero())
        : Base(p0, rho0, lambda)
        , nuclComp_(nuclComp)
        , referenceHeight_(referenceHeight) {}

    inline MassDensityType GetMassDensity(Point const& p) const override;

    inline NuclearComposition const& GetNuclearComposition() const override {
      return nuclComp_;
    }

    inline GrammageType IntegratedGrammage(Trajectory<Line> const& line,
                                           LengthType l) const override;

    inline LengthType ArclengthFromGrammage(Trajectory<Line> const& line,
                                            GrammageType grammage) const override;
  };

} // namespace corsika

#include <corsika/detail/media/SlidingPlanarExponential.inl>
