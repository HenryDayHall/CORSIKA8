/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/BaseExponential.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  template <class T>
  class FlatExponential : public BaseExponential<FlatExponential<T>>, public T
  {
    Vector<units::si::dimensionless_d> const fAxis;
    NuclearComposition const fNuclComp;

    using Base = BaseExponential<FlatExponential<T>>;

  public:

    FlatExponential(Point const& vP0, Vector<units::si::dimensionless_d> const& vAxis,
                    units::si::MassDensityType vRho, units::si::LengthType vLambda, NuclearComposition vNuclComp):
                	Base(vP0, vRho, vLambda),
        fAxis(vAxis),
        fNuclComp(vNuclComp)
     {}

    units::si::MassDensityType GetMassDensity(Point const& vP) const override;

    NuclearComposition const& GetNuclearComposition() const override ;

    units::si::GrammageType IntegratedGrammage(Trajectory<Line> const& vLine,
        units::si::LengthType vTo) const override ;

    units::si::LengthType ArclengthFromGrammage( Trajectory<Line> const& vLine,
        units::si::GrammageType vGrammage) const override ;
  };

} // namespace corsika::environment

#include <corsika/detail/media/FlatExponential.inl>
