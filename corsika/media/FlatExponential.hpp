n/*
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
#include <corsika/media/BaseExponential.hpp>
#include <corsika/media/NuclearComposition.hpp>

namespace corsika {

  template <class T>
  class FlatExponential : public BaseExponential<FlatExponential<T>>, public T {
    Vector<dimensionless_d> const fAxis;
    NuclearComposition const fNuclComp;

    using Base = BaseExponential<FlatExponential<T>>;

  public:
    FlatExponential(Point const& vP0, Vector<dimensionless_d> const& vAxis,
                    MassDensityType vRho, LengthType vLambda,
                    NuclearComposition vNuclComp)
        : Base(vP0, vRho, vLambda)
        , fAxis(vAxis)
        , fNuclComp(vNuclComp) {}

    MassDensityType GetMassDensity(Point const& vP) const override;

    NuclearComposition const& GetNuclearComposition() const override;

    GrammageType IntegratedGrammage(Trajectory<Line> const& vLine,
                                    LengthType vTo) const override;

    LengthType ArclengthFromGrammage(Trajectory<Line> const& vLine,
                                     GrammageType vGrammage) const override;
  };

} // namespace corsika

#include <corsika/detail/media/FlatExponential.inl>
