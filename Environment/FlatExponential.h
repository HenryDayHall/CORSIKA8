/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_FlatExponential_h_
#define _include_Environment_FlatExponential_h_

#include <corsika/environment/BaseExponential.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::environment {

  template <class T>
  class FlatExponential : public BaseExponential<FlatExponential<T>>, public T {
    geometry::Vector<units::si::dimensionless_d> const fAxis;
    NuclearComposition const fNuclComp;

    using Base = BaseExponential<FlatExponential<T>>;

  public:
    FlatExponential(geometry::Point const& p0,
                    geometry::Vector<units::si::dimensionless_d> const& axis,
                    units::si::MassDensityType rho, units::si::LengthType lambda,
                    NuclearComposition nuclComp)
        : Base(p0, rho, lambda)
        , fAxis(axis)
        , fNuclComp(nuclComp) {}

    corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const& p) const override {
      return Base::fRho0 * exp(Base::fInvLambda * (p - Base::fP0).dot(fAxis));
    }

    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::LengthType pTo) const override {
      return Base::IntegratedGrammage(line, pTo, fAxis);
    }

    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::GrammageType pGrammage) const override {
      return Base::ArclengthFromGrammage(line, pGrammage, fAxis);
    }
  };
} // namespace corsika::environment
#endif
