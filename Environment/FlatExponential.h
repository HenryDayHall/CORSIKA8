
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_FlatExponential_h_
#define _include_Environment_FlatExponential_h_

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

#include <cassert>

/**
 *
 */

namespace corsika::environment {

  template <class T>
  class FlatExponential : public T {
    corsika::units::si::MassDensityType const fRho0;
    units::si::LengthType const fLambda;
    units::si::InverseLengthType const fInvLambda;
    NuclearComposition const fNuclComp;
    geometry::Vector<units::si::dimensionless_d> const fAxis;
    geometry::Point const fP0;

  public:
    FlatExponential(geometry::Point const& p0,
                    geometry::Vector<units::si::dimensionless_d> const& axis,
                    units::si::MassDensityType rho, units::si::LengthType lambda,
                    NuclearComposition pNuclComp)
        : fRho0(rho)
        , fLambda(lambda)
        , fInvLambda(1 / lambda)
        , fNuclComp(pNuclComp)
        , fAxis(axis)
        , fP0(p0) {}

    corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const& p) const override {
      return fRho0 * exp(fInvLambda * (p - fP0).dot(fAxis));
    }
    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::LengthType pTo) const override {
      auto const vDotA = line.NormalizedDirection().dot(fAxis).magnitude();

      if (vDotA == 0) {
        return pTo * GetMassDensity(line.GetR0());
      } else {
        return GetMassDensity(line.GetR0()) * (fLambda / vDotA) *
               (exp(vDotA * pTo / fLambda) - 1);
      }
    }

    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::GrammageType pGrammage) const override {
      auto const vDotA = line.NormalizedDirection().dot(fAxis).magnitude();

      if (vDotA == 0) {
        return pGrammage / GetMassDensity(line.GetR0());
      } else {
        return fLambda / vDotA * log(pGrammage * vDotA / (fRho0 * fLambda) + 1);
      }
    }
  };

} // namespace corsika::environment
#endif
